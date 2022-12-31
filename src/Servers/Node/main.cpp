//
// Created by mythi on 12/10/22.
//
#include "NodeManager.h"
#include "EventLoop.h"

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

    auto event_loop = EventLoop::create(runtime->make_worker_thread_executor());

    auto signaling_client = SignalingClient::create(event_loop->event_queue());
    signaling_client->initialize().wait();

    event_loop->append_listener(
            InternalNotificationType::SignalingServerClosed,
            [=](Event) {
                spdlog::warn("Signaling Server Closed...");
                event_loop->end();
            }
    );

    auto manager = NodeManager::create(
            event_loop->event_queue(),
            signaling_client,
            PeerType::Full
    );
    manager->connect_to_peers().wait();
    manager->set_state(PeerState::InitialBlockDownload);

    event_loop->wait();
}
