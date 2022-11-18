#include <chrono>
#include "LightNodeManager.h"
#include "cryptopp/sha.h"
#include "effolkronium/random.hpp"

using namespace krapi;
using namespace CryptoPP;
using namespace std::chrono;
using namespace std::chrono_literals;
using Random = effolkronium::random_local;

int main() {

    LightNodeManager node_manager;
    int message_count = 0;
    SHA256 sha_256;

    // creates a transaction and sends it to all connected peers.

    std::thread thread(
            [&]() {
                auto random = Random();
                while (true) {
                    std::this_thread::sleep_for(5s);
                    auto random_receiver = node_manager.get_random_light_node();

                    if (!random_receiver.has_value()) {
                        spdlog::warn("There are no peers to send transactions to.");
                        continue;
                    }

                    spdlog::info("Main: Sending TX#{} to {}", message_count, random_receiver.value());
                    auto timestamp = (uint64_t) duration_cast<milliseconds>(
                            system_clock::now().time_since_epoch()).count();
                    std::string tx_hash;
                    StringSource s(
                            fmt::format(
                                    "{}{}{}{}{}{}", "send_tx", "tx_status_pending",
                                    timestamp, 0, 1,
                                    random.get(1, 9999)
                            ),
                            true,
                            new HashFilter(sha_256, new HexEncoder(new StringSink(tx_hash)))
                    );
                    node_manager.broadcast_message(
                            PeerMessage{
                                    PeerMessageType::AddTransaction,
                                    Transaction{
                                            TransactionType::Send,
                                            TransactionStatus::Pending,
                                            tx_hash,
                                            timestamp,
                                            node_manager.id(),
                                            random_receiver.value()
                                    }.to_json()
                            }
                    );
                    message_count++;
                }

            }
    );

    node_manager.wait();
}