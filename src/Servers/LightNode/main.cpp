#include <chrono>
#include "LightNodeManager.h"
#include "cryptopp/sha.h"

using namespace krapi;
using namespace CryptoPP;
using namespace std::chrono_literals;

int main() {

    LightNodeManager node_manager;
    int message_count = 0;
    SHA256 sha_256;

    // creates a transaction and sends it to all connected peers.

    std::thread thread(
            [&]() {
                while (true) {
                    spdlog::info("Main: Sending TX#{}", message_count);
                    std::string tx_hash;
                    StringSource s(
                            fmt::format("{}{}{}{}", "send_tx", 0, 1, message_count),
                            true,
                            new HashFilter(sha_256, new HexEncoder(new StringSink(tx_hash)))
                    );
                    node_manager.broadcast_message(
                            PeerMessage{
                                    PeerMessageType::AddTransaction,
                                    Transaction{
                                            TransactionType::Send,
                                            tx_hash,
                                            0,
                                            1
                                    }.to_json()
                            }
                    );
                    message_count++;
                    std::this_thread::sleep_for(5s);
                }

            }
    );

    node_manager.wait();
}