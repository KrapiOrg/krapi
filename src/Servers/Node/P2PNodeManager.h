//
// Created by mythi on 12/11/22.
//

#ifndef SHARED_MODELS_P2PNODEMANAGER_H
#define SHARED_MODELS_P2PNODEMANAGER_H

#include <condition_variable>
#include <future>

#include "spdlog/spdlog.h"

#include "PeerMap.h"
#include "nlohmann/json.hpp"
#include "Message.h"
#include "Response.h"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocket.h"

namespace krapi {

    class P2PNodeManager {

        PeerMap peer_map;
        std::mutex blocking_mutex;
        std::condition_variable blocking_cv;

        rtc::Configuration rtc_config;
        ix::WebSocket ws;
        int my_id;

        static inline std::string peer_state_to_string(rtc::PeerConnection::State state) {

            using namespace rtc;
            switch (state) {
                case PeerConnection::State::New:
                    return "new";
                case PeerConnection::State::Connecting:
                    return "connecting";
                case PeerConnection::State::Connected:
                    return "connected";
                case PeerConnection::State::Disconnected:
                    return "disconnected";
                case PeerConnection::State::Failed:
                    return "failed";
                case PeerConnection::State::Closed:
                    return "closed";
                default:
                    return "unknown";
            }
        }

        static inline std::string gathering_state_to_string(rtc::PeerConnection::GatheringState state) {

            using namespace rtc;
            switch (state) {
                case PeerConnection::GatheringState::New:
                    return "new";
                case PeerConnection::GatheringState::InProgress:
                    return "in-progress";
                case PeerConnection::GatheringState::Complete:
                    return "complete";
                default:
                    return "unknown";
            }
        }

        std::shared_ptr<rtc::PeerConnection> create_connection(int id) {

            auto pc = std::make_shared<rtc::PeerConnection>(rtc_config);

            pc->onStateChange([id](rtc::PeerConnection::State state) {

                spdlog::info("P2PNodeManager: DataChannel with {} is {}", id, peer_state_to_string(state));
            });
            pc->onGatheringStateChange([id](rtc::PeerConnection::GatheringState state) {

                spdlog::info("P2PNodeManager: Gathering for connection with {} is {}", id,
                             gathering_state_to_string(state));
            });

            pc->onLocalDescription([this, id](const rtc::Description &description) {
                auto desc = nlohmann::json{
                        {"id",          id},
                        {"type",        description.typeString()},
                        {"description", std::string(description)}
                };
                auto msg = Message{MessageType::RTCSetup, desc};
                spdlog::info("P2PManager: Sending local description to peer {}", id);
                ws.send(msg);
            });

            pc->onLocalCandidate([this, id](const rtc::Candidate &candidate) {
                auto cand = nlohmann::json{
                        {"id",        id},
                        {"type",      "candidate"},
                        {"candidate", std::string(candidate)},
                        {"mid",       candidate.mid()}
                };
                auto msg = Message{MessageType::RTCSetup, cand};
                //spdlog::info("P2PManager: Sending local candidacy to peer {}", id);
                ws.send(msg);
            });

            pc->onDataChannel([id, this](std::shared_ptr<rtc::DataChannel> remote_channel) {
                spdlog::info("P2PNodeManager: DataChannel from {} received with label {}", id, remote_channel->label());

                remote_channel->onOpen([&, wdc = std::weak_ptr(remote_channel)]() {
                    if (auto dc = wdc.lock())
                        dc->send(fmt::format("Hello from {}", my_id));
                });

                remote_channel->onClosed([id]() {

                    spdlog::info("DataChannel from {} closed", id);
                });

                remote_channel->onMessage([id](auto data) {

                    spdlog::info("P2PNodeManager: Message from {}, received; {}", id, std::get<std::string>(data));
                });
                spdlog::info("Adding channel for {} to map", id);
                peer_map.add_channel(id, remote_channel);
            });
            spdlog::info("Adding peer connection for {} to map", id);
            peer_map.add_peer(id, pc);
            return pc;
        }

        void onWsResponse(const Response &rsp) {

            switch (rsp.type) {

                case ResponseType::PeerAvailable: {

                    auto peer_id = rsp.content.get<int>();
                    spdlog::info("P2PNodeManager: Peer {} is avaliable", peer_id);
                    auto pc = create_connection(peer_id);
                    auto local_channel = pc->createDataChannel("test");

                    local_channel->onOpen([this, peer_id, wdc = std::weak_ptr(local_channel)]() {
                        if (auto dc = wdc.lock())
                            dc->send(fmt::format("Hello from {}", my_id));
                        spdlog::info("DataChannel from {} opened", peer_id);
                    });

                    local_channel->onClosed([peer_id]() {

                        spdlog::info("DataChannel from {} closed", peer_id);
                    });

                    local_channel->onMessage([peer_id](auto data) {

                        spdlog::info("P2PNodeManager: Message from {}, received; {}", peer_id,
                                     std::get<std::string>(data));
                    });

                    peer_map.add_channel(peer_id, local_channel);

                }
                    break;
                case ResponseType::RTCSetup: {

                    auto peer_id = rsp.content["id"].get<int>();
                    auto type = rsp.content["type"].get<std::string>();

                    std::shared_ptr<rtc::PeerConnection> pc;
                    if (peer_map.contains_peer(peer_id)) {

                        pc = peer_map.get_peer(peer_id);
                    } else if (type == "offer") {

                        spdlog::info("Answering to {}", peer_id);
                        pc = create_connection(peer_id);
                    } else {
                        return;
                    }

                    if (type == "offer" || type == "answer") {

                        auto sdp = rsp.content["description"].get<std::string>();
                        pc->setRemoteDescription(rtc::Description(sdp, type));
                    } else if (type == "candidate") {

                        auto sdp = rsp.content["candidate"].get<std::string>();
                        auto mid = rsp.content["mid"].get<std::string>();
                        pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
                    }

                }
                    break;
                default:
                    break;
            }
        }


    public:

        P2PNodeManager() {

            std::promise<int> barrier;
            auto future = barrier.get_future();
            ws.setUrl("ws://127.0.0.1:8080");
            ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr &message) {
                if (message->type == ix::WebSocketMessageType::Open) {

                    ws.send(Message{MessageType::GetIdentitiy});
                } else if (message->type == ix::WebSocketMessageType::Message) {

                    auto response = Response::from_json(message->str);
                    if (response.type == ResponseType::PeerIdentity)
                        barrier.set_value(response.content.get<int>());
                    else
                        spdlog::info("P2PNodeManager: Expected a PeerIdentity response, got {}", response.to_string());
                }
            });
            ws.start();
            spdlog::info("P2PNodeManager: Waiting for identity from signaling server...");
            my_id = future.get();
            spdlog::info("P2PNodeManager: Aquired identity {}", my_id);
            ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr &message) {
                auto msg = Response::from_json(message->str);
                onWsResponse(msg);
            });
        }

        void wait() {

            std::unique_lock l(blocking_mutex);
            blocking_cv.wait(l);
        }

        void boradcast() {

            peer_map.broadcast(fmt::format("FUCK YOU from {}", my_id));
        }

    };

} // krapi

#endif //SHARED_MODELS_P2PNODEMANAGER_H
