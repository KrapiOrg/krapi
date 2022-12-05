#include "effolkronium/random.hpp"

#include "Content/SetTransactionStatusContent.h"
#include "LightNodeManager.h"
#include "Wallet.h"
#include "spdlog/spdlog.h"

using namespace krapi;
using namespace std::chrono_literals;
using Random = effolkronium::random_static;


int main() {

    auto manager = std::make_shared<LightNodeManager>();
    Wallet wallet;

    manager->append_listener(
            PeerMessageType::SetTransactionStatus,
            [&](PeerMessage message) {

                auto content = SetTransactionStatusContent::from_json(message.content());
                wallet.set_transaction_status(content.status(), content.hash());
            }
    );

    manager->append_listener(
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
            [&manager](Transaction transaction, TransactionStatus before, TransactionStatus after) {

                spdlog::info(
                        "Main: {} status changed from {} to {}",
                        transaction.hash().substr(0, 10),
                        to_string(before),
                        to_string(after)
                );
                // TODO: Implement Transaction handling correctly
            }
    );

    spdlog::info("Trying to connect to network");
    auto peers_connected_to = manager->connect_to_peers().get();
    spdlog::info("Connected to [{}]", fmt::join(peers_connected_to, ", "));

    while (true) {
        std::cin.get();
        auto random_receiver = manager->random_light_node();

        if (!random_receiver.has_value()) {
            spdlog::warn("{}", random_receiver.error().err_str);
            continue;
        }
        auto random_id = random_receiver.value();
        auto tx = wallet.create_transaction(
                manager->id(),
                random_id
        );

        spdlog::info(
                "Main: Sending TX {} to {}",
                tx.hash().substr(0, 10),
                random_id
        );
        (void) manager->broadcast(
                PeerMessage{
                        PeerMessageType::AddTransaction,
                        manager->id(),
                        tx.to_json()
                }
        );

    }
}