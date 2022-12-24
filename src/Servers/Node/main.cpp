//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "EventQueue.h"

using namespace krapi;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {

    concurrencpp::runtime runtime;

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
    auto event_queue = EventQueue::create();
    auto event_loop = runtime.timer_queue()->make_timer(
            0s,
            30ms,
            runtime.make_worker_thread_executor(),
            [&]() {
                event_queue->process_one();
            }
    );
    auto manager = std::make_shared<NodeManager>(
            make_not_null(event_queue.get()),
            PeerType::Full
    );
    manager->initialize().wait();
    auto peers = manager->connect_to_peers().get();
    spdlog::info("Main: Connected to [{}]", fmt::join(peers,", "));
}
