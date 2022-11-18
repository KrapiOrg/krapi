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

    int message_count = 0;


    node_manager.append_listener(
            PeerMessageType::SetTransactionStatus,
            [&](PeerMessage message) {

                auto content = SetTransactionStatusContent::from_json(message.content);
                wallet.set_transaction_status(content.status(), content.hash());
            }
    );

    node_manager.append_listener(
            PeerMessageType::AddTransaction,
            [&](PeerMessage message) {

                auto transaction = Transaction::from_json(message.content);

                spdlog::info("Main: Received transaction {}", transaction.to_json().dump(4));
                wallet.add_transaction(transaction);
            }
    );

    wallet.append_listener(
            Wallet::Event::TransactionStatusChanged,
            [](Transaction transaction, TransactionStatus before, TransactionStatus after) {

                spdlog::info(
                        "Transaction {} status changed from {} to {}",
                        transaction.hash().substr(0, 6),
                        to_string(before),
                        to_string(after)
                );
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
                        std::this_thread::sleep_for(5s);
                        auto random_receiver = node_manager.get_random_light_node();

                        if (!random_receiver.has_value()) {
                            spdlog::warn("There are no peers to send transactions to.");
                            continue;
                        }

                        spdlog::info("Main: Sending TX#{} to {}", message_count, random_receiver.value());

                        node_manager.broadcast_message(
                                PeerMessage{
                                        PeerMessageType::AddTransaction,
                                        node_manager.id(),
                                        wallet.create_transaction(
                                                node_manager.id(),
                                                random_receiver.value()
                                        ).to_json()
                                }
                        );
                        message_count++;
                    }

                }
        );
    } else {
        spdlog::info("This node does not send transactions!");
    }
    (void) thread;
    node_manager.wait();
}