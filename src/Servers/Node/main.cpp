//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "EventQueue.h"

using namespace krapi;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {

    auto runtime = std::make_shared<concurrencpp::runtime>();

    constexpr int BATCH_SZE = 10;
    std::string path;

    if (argc == 2) {
        path = std::string{argv[1]};
    } else {
        path = "blockchain";
    }

    if (!fs::exists(path)) {
        spdlog::error("Could not find {}, creating...", path);
        fs::create_directory(path);
    }

    auto end = std::make_shared<std::atomic<bool>>(false);
    auto event_queue = EventQueue::create();

    event_queue->append_listener(
            InternalNotificationType::SignalingServerClosed,
            [=](Event) {
                spdlog::warn("Signaling Server Closed...");
                end->exchange(true);
            }
    );
    auto worker = runtime->thread_executor();
    auto event_loop = worker->submit(
            [=]() {
                while (!end->load()) {
                    event_queue->wait();
                    event_queue->process_one();
                }
            }
    );
    auto manager = NodeManager::create(
            event_queue,
            PeerType::Full
    );

    event_loop.wait();
}
