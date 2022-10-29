//
// Created by mythi on 13/10/22.
//

#include "NetworkConnection.h"

#include <utility>
#include "spdlog/spdlog.h"

namespace krapi {

    NetworkConnection::NetworkConnection(
            std::string uri,
            NodeMessageQueuePtr eq
    ) : m_uri(std::move(uri)),
        m_eq(std::move(eq)),
        m_identity(-1) {

        spdlog::info("Trying to connect to {}", m_uri);

        m_thread = std::jthread(&NetworkConnection::server_loop, this);

        auto identity_future = identity_promise.get_future();
        identity_future.wait();
        m_identity = identity_future.get();


        assert(m_identity != -1);
        spdlog::info("Connected to Node with Identity {}", m_identity);
    }

    void NetworkConnection::server_loop() {

        using namespace ix;

        WebSocket socket;
        socket.setUrl(fmt::format("ws://{}", m_uri));

        socket.setOnMessageCallback(
                [&, this](const WebSocketMessagePtr &message) {
                    if (message->type == WebSocketMessageType::Open) {
                        spdlog::info("Opened connection to {}", socket.getUrl());
                        spdlog::info("Sending Identity Request to {}", socket.getUrl());
                        auto msg = NodeMessage{NodeMessageType::NodeIdentityRequest};
                        auto msg_json = nlohmann::json(msg);
                        socket.send(msg_json.dump());
                    } else if (message->type == WebSocketMessageType::Error) {
                        spdlog::error("{}, {}", socket.getUrl(), message->errorInfo.reason);
                    } else if (message->type == WebSocketMessageType::Message) {
                        auto msg = NodeMessage{};
                        nlohmann::from_json(nlohmann::json::parse(message->str), msg);

                        if (msg.type == NodeMessageType::NodeIdentityReply) {
                            auto identity = msg.content.get<int>();
                            identity_promise.set_value(identity);
                            this->m_identity = identity;
                        } else if (msg.type == NodeMessageType::Stop) {
                            socket.disableAutomaticReconnection();
                            socket.stop();
                        } else {

                            m_eq->dispatch(msg.type, msg);
                        }
                    }
                }
        );
        m_eq->appendListener(
                NodeMessageType::BroadcastTx,
                [&, this](const NodeMessage &msg) {
                    if (msg.receiver_identity == m_identity) {
                        spdlog::info("Forwarding broadcast message to {}", m_identity);
                        auto bmsg = msg;
                        bmsg.blacklist.push_back(m_uri);
                        auto bmsg_json = nlohmann::json(bmsg);
                        socket.send(bmsg_json.dump());
                    }
                }
        );
        socket.run();
    }

    void NetworkConnection::wait() {

        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    void NetworkConnection::stop() {

        if (m_thread.joinable()) {
            m_eq->dispatch(NodeMessageType::Stop, NodeMessage{});
            m_thread.request_stop();
            m_thread.join();
        }
    }

    int NetworkConnection::identity() {

        return m_identity;
    }


}