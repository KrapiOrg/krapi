//
// Created by mythi on 09/11/22.
//

#include <unordered_set>
#include <utility>
#include "spdlog/spdlog.h"
#include "ixwebsocket/IXWebSocketServer.h"
#include "ixwebsocket/IXSetThreadName.h"
#include "SignalingMessage.h"


using namespace krapi;

namespace krapi {
    class WebSocket : public ix::WebSocket {
        static int _id;
        int m_id;
    public:
        WebSocket() : m_id(_id++) {

        }

        [[nodiscard]]
        int id() const {

            return m_id;
        }
    };

    int WebSocket::_id = 0;

    class WebSocketServer : public ix::SocketServer {

        using OnMessageCallback = std::function<void(std::shared_ptr<ix::ConnectionState> connectionState, WebSocket &,
                                                     const SignalingMessage &)>;
        OnMessageCallback m_onMessage;

        std::mutex m_clients_mutex;
        std::unordered_map<int, std::shared_ptr<WebSocket>> m_clients;

        void handleConnection(
                std::unique_ptr<ix::Socket> socket,
                std::shared_ptr<ix::ConnectionState> connectionState
        ) override {

            ix::setThreadName("Srv:ws:" + connectionState->getId());

            auto ws = std::make_shared<WebSocket>();
            auto wsRaw = ws.get();

            ws->setOnMessageCallback(
                    [this, wsRaw, connectionState](const ix::WebSocketMessagePtr &msg) {
                        if (msg->type == ix::WebSocketMessageType::Open) {

                            spdlog::info("WebSocketServer: {} just opened a connection", wsRaw->id());
                        } else if (msg->type == ix::WebSocketMessageType::Message) {

                            m_onMessage(connectionState, *wsRaw,
                                        SignalingMessage::from_json(nlohmann::json::parse(msg->str)));
                        }
                    }
            );

            ws->disableAutomaticReconnection();

            {
                std::lock_guard<std::mutex> lock(m_clients_mutex);
                m_clients.insert({ws->id(), ws});
            }

            auto status = ws->connectToSocket(std::move(socket), 3, true);
            if (status.success) {

                ws->run();
            } else {
                spdlog::error(
                        "WebSocketServer: handleConnection() HTTP status: {}, error: {}",
                        status.http_status,
                        status.errorStr
                );

            }

            ws->setOnMessageCallback(nullptr);

            // Remove this client from our client set
            {
                std::lock_guard<std::mutex> lock(m_clients_mutex);
                if (m_clients.erase(ws->id()) != 1) {
                    spdlog::error("WebSocketServer: Failed to delete client");
                }
            }
            connectionState->setTerminated();
            spdlog::info("WebSocketServer: {} has disconnected", ws->id());
        }

        size_t getConnectedClientsCount() override {

            return clients().size();
        }

    public:
        void set_on_message(OnMessageCallback onMessage) {

            m_onMessage = std::move(onMessage);
        }

        std::optional<std::shared_ptr<WebSocket>> client(int id) {

            auto cs = clients();
            if (cs.contains(id)) {
                return cs[id];
            }
            return {};
        }

        void peer_update(int p) {

            for (const auto &[id, ws]: clients()) {
                if (id != p) {
                    ws->send(SignalingMessage{SignalingMessageType::PeerAvailable, p}.to_string());
                    do {
                        std::chrono::duration<double, std::milli> duration(500);
                        std::this_thread::sleep_for(duration);
                    } while (ws->bufferedAmount() != 0);
                }
            }
        }

        std::unordered_map<int, std::shared_ptr<WebSocket>> clients() {

            std::lock_guard l(m_clients_mutex);
            return m_clients;
        }

        std::unordered_set<int> client_ids() {

            auto cs = clients();
            auto ids = std::unordered_set<int>{};
            for (const auto &kv: cs) {
                ids.insert(kv.first);
            }
            return ids;
        }

        std::pair<bool, std::string> listen_and_start() {

            auto res = listen();
            start();
            return res;
        }

    };

};

int main(int argc, char **argv) {

    krapi::WebSocketServer server;
    server.set_on_message(
            [&](
                    std::shared_ptr<ix::ConnectionState> state,
                    krapi::WebSocket &ws,
                    const krapi::SignalingMessage &message
            ) {
                if (message.type == SignalingMessageType::IdentityRequest) {
                    auto response = SignalingMessage{SignalingMessageType::IdentityResponse, ws.id()};
                    ws.send(response.to_string());
                    server.peer_update(ws.id());
                } else if (message.type == SignalingMessageType::AvailablePeersRequest) {
                    auto client_ids = server.client_ids();
                    client_ids.erase(ws.id());
                    auto response = SignalingMessage{
                            SignalingMessageType::AvailablePeersResponse,
                            std::move(client_ids)
                    };
                    ws.send(response.to_string());
                } else if (message.type == SignalingMessageType::RTCSetup) {
                    auto clients = server.clients();
                    auto id = message.content["id"].get<int>();

                    if (clients.contains(id)) {

                        auto response = SignalingMessage{SignalingMessageType::RTCSetup, message.content};
                        response.content["id"] = ws.id();
                        clients[id]->send(response.to_json().dump());
                    }

                }
            }
    );

    auto res = server.listen_and_start();

    if (!res.first) {
        spdlog::error("Failed to start server {}", res.second);
        return 1;
    }
    spdlog::info("Started WebSocket server on port {}", server.getPort());
    server.wait();
}