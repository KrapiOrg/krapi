#include "BlockchainManager.h"

namespace krapi {

  void BlockchainManager::on_block_headers_request(Event e) {
    auto peer_message = e.get<PeerMessage>();
    auto last_header = BlockHeader::from_json(peer_message->content());

    m_peer_manager->send_and_forget(
      PeerMessageType::BlockHeadersResponse,
      peer_message->receiver_identity(),
      peer_message->sender_identity(),
      peer_message->tag(),
      BlockHeadersResponseContent(
        m_blockchain->get_all_after(last_header)
      )
        .to_json()
    );
  }
  void BlockchainManager::on_block_request(Event e) {
    auto peer_message = e.get<PeerMessage>();
    auto block_header = BlockHeader::from_json(peer_message->content());
    spdlog::info(
      "{} requested block {}",
      peer_message->sender_identity(),
      block_header.contrived_hash()
    );
    auto block = m_blockchain->get(block_header.hash());
    if (block) {

      m_peer_manager->send_and_forget(
        PeerMessageType::BlockResponse,
        m_peer_manager->id(),
        peer_message->sender_identity(),
        peer_message->tag(),
        block->to_json()
      );
    } else {

      m_peer_manager->send_and_forget(
        PeerMessageType::BlockNotFoundResponse,
        m_peer_manager->id(),
        peer_message->sender_identity(),
        peer_message->tag()
      );
    }
  }
  void BlockchainManager::on_add_block(Event e) {
    auto message = e.get<PeerMessage>();
    auto block = Block::from_json(message->content());


    spdlog::info(
      "Validating block #{} from peer {}",
      block.contrived_hash(),
      message->sender_identity()
    );

    auto validation_state = Validator::validate_block(
      m_blockchain,
      m_spent_transactions_store,
      block
    );

    if (validation_state != ValidationState::Success) {

      spdlog::warn(
        "Failed to validation block #{}, {}",
        block.contrived_hash(),
        to_string(validation_state)
      );

      m_peer_manager->send_and_forget(
        PeerMessageType::BlockRejected,
        m_peer_manager->id(),
        message->sender_identity(),
        PeerMessage::create_tag(),
        nlohmann::json(validation_state)
      );
      return;
    }

    spdlog::info(
      "Successfully Validated Block #{} from peer {}",
      block.contrived_hash(),
      message->sender_identity()
    );

    m_blockchain->put(block);

    spdlog::info(
      "Removing transactions in Block#{} from pool",
      block.contrived_hash()
    );

    for (const auto &transaction: block.transactions()) {

      if (m_transaction_pool->remove(transaction)) {
        spdlog::info("  Removed Tx#{} from pool", transaction.contrived_hash());
        m_spent_transactions_store->put(transaction);
      }
    }

    m_peer_manager->send_and_forget(
      PeerMessageType::BlockAccepted,
      m_peer_manager->id(),
      message->sender_identity(),
      PeerMessage::create_tag(),
      block.header().to_json()
    );
  }
  void BlockchainManager::on_block_mined(Event e) {
    auto block = e.get<InternalNotification<Block>>()->content();
    auto validation_state = Validator::validate_block(
      m_blockchain,
      m_spent_transactions_store,
      block
    );

    spdlog::info("Validating mined block #{}", block.contrived_hash());
    if (validation_state != ValidationState::Success) {

      spdlog::warn(
        "Failed to validate mined block #{}, {}",
        block.contrived_hash(),
        to_string(validation_state)
      );
      m_event_loop->event_queue()->enqueue<InternalNotification<void>>(
        InternalNotificationType::MinedBlockValidated
      );
      return;
    }

    spdlog::info(
      "Adding Mined Block#{} to blockchain",
      block.contrived_hash()
    );

    m_blockchain->put(block);

    spdlog::info(
      "Removing transactions within {} block from transaction pool",
      block.contrived_hash()
    );

    for (const auto &transaction: block.transactions()) {

      if (m_transaction_pool->remove(transaction)) {
        spdlog::info("    Removed Tx#{} from pool", transaction.contrived_hash());
        m_spent_transactions_store->put(transaction);
      }
    }

    m_event_loop->event_queue()->enqueue<InternalNotification<void>>(
      InternalNotificationType::MinedBlockValidated
    );
  }
  BlockchainManager::BlockchainManager(
    EventLoopPtr event_loop,
    PeerManagerPtr peer_manager,
    TransactionPoolPtr transaction_pool,
    BlockchainPtr blockchain,
    SpentTransactionsStorePtr spent_transactions_store
  )
      : m_event_loop(std::move(event_loop)),
        m_peer_manager(std::move(peer_manager)),
        m_transaction_pool(std::move(transaction_pool)),
        m_blockchain(std::move(blockchain)),
        m_spent_transactions_store(std::move(spent_transactions_store)) {

    m_event_loop->append_listener(
      PeerMessageType::BlockHeadersRequest,
      std::bind_front(&BlockchainManager::on_block_headers_request, this)
    );
    m_event_loop->append_listener(
      PeerMessageType::BlockRequest,
      std::bind_front(&BlockchainManager::on_block_request, this)
    );
    m_event_loop->append_listener(
      PeerMessageType::AddBlock,
      std::bind_front(&BlockchainManager::on_add_block, this)
    );

    m_event_loop->append_listener(
      InternalNotificationType::BlockMined,
      std::bind_front(&BlockchainManager::on_block_mined, this)
    );
  }
}// namespace krapi