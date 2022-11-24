//
// Created by mythi on 24/11/22.
//

#include <sstream>
#include "NodeManager.h"
#include "Message.h"

#include "spdlog/spdlog.h"

namespace krapi {
    std::shared_ptr<rtc::PeerConnection> NodeManager::create_connection(int peer_id) {

        auto pc = std::make_shared<rtc::PeerConnection>(rtc_config);

        pc->onStateChange(
                [peer_id](rtc::PeerConnection::State state) {
                    std::stringstream ss;
                    ss << state;
                    spdlog::info("PeerConnection to {} state: {}", peer_id, ss.str());
                }
        );
        pc->onGatheringStateChange([peer_id](rtc::PeerConnection::GatheringState state) {
            std::stringstream ss;
            ss << state;
            spdlog::info("PeerConnection to {} gathering state: {}", peer_id, ss.str());
        });

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

        pc->onDataChannel([this, peer_id](std::shared_ptr<rtc::DataChannel> answerer_channel) {

            answerer_channel->onClosed([]() {
                spdlog::info("ANSWERER CHANNEL CLOSED");
            });
            answerer_channel->onError([](std::string err) {
                spdlog::info("ANSWERER Channel ERROR:", err);
            });
            peer_map.add_channel(
                    peer_id,
                    answerer_channel,
                    [this](PeerMessage message) {
                        m_dispatcher.dispatch(message.type(), message);
                    }
            );
        });

        peer_map.add_peer(peer_id, pc);
        return pc;
    }

    void NodeManager::onWsResponse(const Response &rsp) {

        switch (rsp.type) {

            case ResponseType::PeerAvailable: {

                auto peer_id = rsp.content.get<int>();
                spdlog::info("NodeManager: Peer {} is avaliable", peer_id);
                auto pc = create_connection(peer_id);
                auto offerer_channel = pc->createDataChannel("krapi");
                offerer_channel->onClosed([]() {
                    spdlog::info("OFFERER CHANNEL CLOSED");
                });
                offerer_channel->onError([](std::string err) {
                    spdlog::info("OFFERER Channel ERROR:", err);
                });
                peer_map.add_channel(
                        peer_id,
                        offerer_channel,
                        [this](PeerMessage message) {
                            m_dispatcher.dispatch(message.type(), message);
                        }
                );
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

    NodeManager::NodeManager(PeerType peer_type) {


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
        spdlog::info("NodeManager: Aquired identity {}", my_id);
        ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr &message) {
            auto msg = Response::from_json(message->str);
            onWsResponse(msg);
        });

        m_dispatcher.appendListener(
                PeerMessageType::PeerTypeRequest,
                [this, peer_type](PeerMessage message) {

                    send_message(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::PeerTypeResponse,
                                    id(),
                                    message.tag(),
                                    peer_type
                            }
                    );
                }
        );
    }

    void NodeManager::wait() {

        std::unique_lock l(blocking_mutex);
        blocking_cv.wait(l);
    }

    void NodeManager::broadcast_message(const PeerMessage &message) {

        peer_map.broadcast(message);
    }

    void NodeManager::append_listener(PeerMessageType type, std::function<void(PeerMessage)> listener) {

        m_dispatcher.appendListener(type, listener);
    }

    int NodeManager::id() const {

        return my_id;
    }

    std::shared_future<PeerMessage> NodeManager::send_message(
            int id,
            PeerMessage message,
            std::optional<PeerMessageCallback> callback
    ) {

        return peer_map.send_message(id, std::move(message), std::move(callback));
    }
} // krapi