//
// Created by mythi on 13/10/22.
//

#include "NodeServer.h"

#include <utility>
#include "spdlog/spdlog.h"
#include "ServerMessageHandeler.h"

namespace krapi {

    NodeServer::NodeServer(
            std::string uri,
            std::shared_ptr<eventpp::EventDispatcher<NodeMessageType, void(const NodeMessage &)>> eq
    ) : m_uri(std::move(uri)),
        m_eq(std::move(eq)) {

        start();
    }

    void NodeServer::server_loop() {

        using namespace ix;
        WebSocket socket;
        socket.setUrl(fmt::format("ws://{}", m_uri));

        socket.setOnMessageCallback(
                [&, this](const WebSocketMessagePtr &message) {
                    if (message->type == WebSocketMessageType::Open) {
                        spdlog::info("Opened connection to {}", socket.getUrl());
                    } else if (message->type == WebSocketMessageType::Error) {
                        spdlog::error("{}, {}", socket.getUrl(), message->errorInfo.reason);
                    } else if (message->type == WebSocketMessageType::Message) {
                        auto msg = NodeMessage{};
                        nlohmann::from_json(nlohmann::json::parse(message->str), msg);
                        //spdlog::info("Received message {}", msg.str);
                        // Append message to queue
                        m_eq->dispatch(msg.type, msg);
                    }
                }
        );
        m_eq->appendListener(NodeMessageType::Stop, [&](const NodeMessage &) {
            socket.disableAutomaticReconnection();
            socket.stop();
        });
        socket.run();
    }

    void NodeServer::start() {

        spdlog::warn("Starting node server for {}", m_uri);
        m_thread = std::jthread(&NodeServer::server_loop, this);

    }

    void NodeServer::wait() {

        spdlog::warn("Here2");
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    void NodeServer::stop() {

        if (m_thread.joinable()) {
            m_eq->dispatch(NodeMessageType::Stop, NodeMessage{});
            m_thread.request_stop();
            m_thread.join();
        }
    }


}