#include <chrono>

#include "effolkronium/random.hpp"

#include "LightNodeManager.h"
#include "Wallet.h"

using namespace krapi;
using namespace std::chrono_literals;
using Random = effolkronium::random_static;


int main() {

    LightNodeManager node_manager;
    Wallet wallet;

    node_manager.append_listener(
            PeerMessageType::SetTransactionStatus,
            [&](PeerMessage message) {

                auto content = SetTransactionStatusContent::from_json(message.content());
                wallet.set_transaction_status(content.status(), content.hash());
            }
    );

    node_manager.append_listener(
            PeerMessageType::AddTransaction,
            [&](PeerMessage message) {

                auto transaction = Transaction::from_json(message.content());

                if (wallet.add_transaction(transaction)) {
                    spdlog::info("Main: Added {} to wallet", transaction.hash().substr(0, 10));
                }
            }
    );

    wallet.append_listener(
            Wallet::Event::TransactionStatusChanged,
            [&node_manager](Transaction transaction, TransactionStatus before, TransactionStatus after) {

                spdlog::info(
                        "Main: {} status changed from {} to {}",
                        transaction.hash().substr(0, 10),
                        to_string(before),
                        to_string(after)
                );
                if (after == TransactionStatus::Rejected) {
                    spdlog::info("Main: Rebroadcasting {}", transaction.hash().substr(0, 10));
                    transaction.set_status(TransactionStatus::Pending);
                    node_manager.broadcast_message(
                            PeerMessage{
                                    PeerMessageType::AddTransaction,
                                    node_manager.id(),
                                    transaction.to_json()
                            }
                    );
                }
            }
    );

    auto shouldBeASender = Random::get(0, 1);
    // If we are a sender, create transactions and send them to all connected peers.
    std::thread thread;
    if (shouldBeASender == 1) {

        spdlog::info("This node sends transactions!");
        thread = std::thread(
                [&]() {

                    while (true) {
                        std::this_thread::sleep_for(1s);
                        auto random_receiver = node_manager.get_random_light_node();

                        if (!random_receiver.has_value()) {
                            spdlog::warn("There are no peers to send transactions to.");
                            continue;
                        }
                        auto tx = wallet.create_transaction(
                                node_manager.id(),
                                random_receiver.value()
                        );
                        spdlog::info(
                                "Main: Sending TX {} to {}", tx.hash().substr(0, 10), random_receiver.value()
                        );

                        node_manager.broadcast_message(
                                PeerMessage{
                                        PeerMessageType::AddTransaction,
                                        node_manager.id(),
                                        PeerMessage::create_tag(),
                                        tx.to_json()
                                }
                        );
                    }

                }
        );
    } else {
        spdlog::info("This node does not send transactions!");
    }
    (void) thread;
    node_manager.wait();
}