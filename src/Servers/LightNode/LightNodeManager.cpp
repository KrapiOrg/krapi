//
// Created by mythi on 12/11/22.
//

#include "LightNodeManager.h"
#include "PeerType.h"
#include "effolkronium/random.hpp"

namespace krapi {
    std::shared_ptr<rtc::PeerConnection> LightNodeManager::create_connection(int peer_id) {

        auto pc = std::make_shared<rtc::PeerConnection>(rtc_config);

        pc->onLocalDescription([this, peer_id](const rtc::Description &description) {
            auto desc = nlohmann::json{
                    {"id",          peer_id},
                    {"type",        description.typeString()},
                    {"description", std::string(description)}
            };
            auto msg = Message{MessageType::RTCSetup, desc};
            ws.send(msg);
        });

        pc->onLocalCandidate([this, peer_id](const rtc::Candidate &candidate) {
            auto cand = nlohmann::json{
                    {"id",        peer_id},
                    {"type",      "candidate"},
                    {"candidate", std::string(candidate)},
                    {"mid",       candidate.mid()}
            };
            auto msg = Message{MessageType::RTCSetup, cand};
            ws.send(msg);
        });

        pc->onDataChannel([peer_id, this](std::shared_ptr<rtc::DataChannel> answerer_channel) {
            answerer_channel->onOpen([acw = std::weak_ptr(answerer_channel)]() {

                if (auto ac = acw.lock())
                    ac->send(PeerMessage{PeerMessageType::PeerTypeRequest});
            });
            answerer_channel->onMessage([peer_id, this](auto data) {
                auto str_data = std::get<std::string>(data);
                onRemoteMessage(peer_id, PeerMessage::from_json(str_data));
            });
            peer_map.add_channel(peer_id, answerer_channel);
        });

        peer_map.add_peer(peer_id, pc);
        return pc;
    }

    void LightNodeManager::onWsResponse(const Response &rsp) {

        switch (rsp.type) {

            case ResponseType::PeerAvailable: {

                auto peer_id = rsp.content.get<int>();
                spdlog::info("NodeManager: Peer {} is available", peer_id);
                auto pc = create_connection(peer_id);
                auto offerer_channel = pc->createDataChannel("krapi");
                offerer_channel->onOpen([woc = std::weak_ptr(offerer_channel)]() {

                    if (auto offerer_channel = woc.lock())
                        offerer_channel->send(PeerMessage{PeerMessageType::PeerTypeRequest});
                });
                offerer_channel->onMessage([this, peer_id](auto data) {
                    auto str_data = std::get<std::string>(data);
                    onRemoteMessage(peer_id, PeerMessage::from_json(str_data));
                });

                peer_map.add_channel(peer_id, offerer_channel);
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

    LightNodeManager::LightNodeManager() {

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
                    spdlog::info("NodeManager: Expected a PeerIdentity response, got {}", response.to_string());
            }
        });
        ws.start();
        spdlog::info("NodeManager: Waiting for identity from signaling server...");
        my_id = future.get();
        spdlog::info("NodeManager: Acquired identity {}", my_id);
        ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr &message) {
            auto msg = Response::from_json(message->str);
            onWsResponse(msg);
        });
    }

    void LightNodeManager::wait() {

        std::unique_lock l(blocking_mutex);
        blocking_cv.wait(l);
    }

    void LightNodeManager::onRemoteMessage(int id, PeerMessage message) {


        if (message.type == PeerMessageType::PeerTypeRequest) {

            spdlog::info("NodeManager: PeerType Requested");
            auto channel = peer_map.get_channel(id);
            channel->send(
                    PeerMessage{
                            PeerMessageType::PeerTypeResponse,
                            my_id,
                            PeerType::Light
                    }
            );
        } else if (message.type == PeerMessageType::PeerTypeResponse) {
            auto peer_type = message.content.get<PeerType>();

            spdlog::info("NodeManager: Setting PeerType of {} to {}", id, message.content.dump());
            peer_map.set_peer_type(id, peer_type);
        }

        m_dispatcher.dispatch(message.type, message);
    }

    void LightNodeManager::broadcast_message(PeerMessage message) {

        peer_map.broadcast(std::move(message));
    }

    int LightNodeManager::id() const {

        return my_id;
    }

    std::optional<int> LightNodeManager::get_random_light_node() {

        using Random = effolkronium::random_static;

        auto ids = peer_map.get_light_node_ids();
        if (!ids.empty()) {

            auto random_index = Random::get(0, static_cast<int>(ids.size()) - 1);
            return ids[random_index];
        }
        return {};
    }

    void LightNodeManager::append_listener(
            PeerMessageType message_type,
            std::function<void(PeerMessage)> callback
    ) {

        m_dispatcher.appendListener(message_type, callback);
    }
} // krapi