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


int main(int argc, char **argv) {

    std::mutex mutex;
    std::unordered_map<const ix::WebSocket *, std::string> ws_to_identities;
    ix::WebSocketServer server;

    server.setOnClientMessageCallback(
            [&](
                    const std::shared_ptr<ix::ConnectionState> &state,
                    ix::WebSocket &ws,
                    const ix::WebSocketMessagePtr &message
            ) {
                if (state->isTerminated()
                    || ws.getReadyState() == ix::ReadyState::Closed
                    || ws.getReadyState() == ix::ReadyState::Closing
                    || message->type == ix::WebSocketMessageType::Error
                    || message->type == ix::WebSocketMessageType::Close) {

                    auto id_itr = ws_to_identities.find(&ws);
                    if (id_itr != ws_to_identities.end()) {
                        auto id_str = id_itr->second;
                        if (ws_to_identities.erase(&ws) == 1) {
                            spdlog::warn("{} node just disconnected", id_str);
                            return;
                        }
                    }
                }
                if (message->type == ix::WebSocketMessageType::Message) {

                    auto signalingMessage = krapi::SignalingMessage::from_json(nlohmann::json::parse(message->str));

                    if (signalingMessage.type == SignalingMessageType::SetIdentityRequest
                        || signalingMessage.type == SignalingMessageType::IdentityResponse) {
                        auto identity = signalingMessage.content.get<std::string>();
                        spdlog::info("{} Just Connected", identity);
                        {
                            std::lock_guard l(mutex);
                            ws_to_identities[&ws] = identity;
                        }
                        ws.send(
                                SignalingMessage{
                                        SignalingMessageType::IdentityAcknowledged,
                                        signalingMessage.tag,
                                        "identity_acknowledged"
                                }.to_string()
                        );
                    } else if (signalingMessage.type == SignalingMessageType::AvailablePeersRequest) {

                        std::vector<std::string> identities;
                        for (const auto &client: server.getClients()) {

                            auto clientPtr = &*client;
                            if (clientPtr == &ws)
                                continue;
                            if (ws_to_identities.contains(clientPtr)) {
                                std::lock_guard l(mutex);
                                identities.push_back(ws_to_identities.find(clientPtr)->second);
                            } else {
                                client->send(
                                        SignalingMessage{
                                                SignalingMessageType::IdentityRequest
                                        }.to_string()
                                );
                            }
                        }

                        auto response = SignalingMessage{
                                SignalingMessageType::AvailablePeersResponse,
                                signalingMessage.tag,
                                std::move(identities)
                        };
                        ws.send(response.to_string());
                    } else if (signalingMessage.type == SignalingMessageType::RTCSetup) {

                        auto clients = server.getClients();

                        auto identity = signalingMessage.content["id"].get<std::string>();
                        for (const auto &client: clients) {

                            auto clientPtr = &*client;
                            if (clientPtr == &ws)
                                continue;

                            std::string sid;

                            {
                                std::lock_guard l(mutex);
                                auto id = ws_to_identities.find(clientPtr);
                                if (id == ws_to_identities.end()) {
                                    ws.send(SignalingMessage{SignalingMessageType::IdentityRequest}.to_string());
                                    continue;
                                } else {
                                    sid = id->second;
                                }
                            }

                            if (sid == identity) {

                                auto response = SignalingMessage{
                                        SignalingMessageType::RTCSetup,
                                        signalingMessage.tag,
                                        signalingMessage.content
                                };
                                {
                                    std::lock_guard l(mutex);
                                    response.content["id"] = ws_to_identities[&ws];
                                }
                                client->send(response.to_string());
                            }

                        }

                    }
                }

            }
    );

    auto res = server.listenAndStart();

    if (!res) {
        spdlog::error("Failed to start server");
        return 1;
    }
    spdlog::info("Started WebSocket server on port {}", server.getPort());
    server.wait();
}