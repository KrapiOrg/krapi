//
// Created by mythi on 24/11/22.
//

#include <sstream>
#include "NodeManager.h"
#include "spdlog/spdlog.h"

namespace krapi {
    std::shared_ptr<rtc::PeerConnection> NodeManager::create_connection(int peer_id) {

        auto pc = std::make_shared<rtc::PeerConnection>(rtc_config);

        pc->onLocalDescription([this, peer_id](const rtc::Description &description) {
            auto desc = nlohmann::json{
                    {"id",          peer_id},
                    {"type",        description.typeString()},
                    {"description", std::string(description)}
            };
            auto msg = SignalingMessage{SignalingMessageType::RTCSetup, desc};
            ws.send(msg);
        });

        pc->onLocalCandidate([this, peer_id](const rtc::Candidate &candidate) {
            auto cand = nlohmann::json{
                    {"id",        peer_id},
                    {"type",      "candidate"},
                    {"candidate", std::string(candidate)},
                    {"mid",       candidate.mid()}
            };
            auto msg = SignalingMessage{SignalingMessageType::RTCSetup, cand};
            ws.send(msg);
        });

        pc->onDataChannel([this, peer_id](
                const std::shared_ptr<rtc::DataChannel> &answerer_channel
        ) {

            add_channel(
                    peer_id,
                    answerer_channel
            );
        });

        add_peer_connection(peer_id, pc);
        return pc;
    }

    void NodeManager::on_signaling_message(const SignalingMessage &rsp) {

        switch (rsp.type) {

            case SignalingMessageType::PeerAvailable: {

                auto peer_id = rsp.content.get<int>();
                auto pc = create_connection(peer_id);
                auto offerer_channel = pc->createDataChannel("krapi");

                add_channel(
                        peer_id,
                        offerer_channel
                );
            }
                break;
            case SignalingMessageType::RTCSetup: {

                auto peer_id = rsp.content["id"].get<int>();
                auto type = rsp.content["type"].get<std::string>();

                std::shared_ptr<rtc::PeerConnection> pc;
                if (peer_map.contains(peer_id)) {

                    pc = peer_map[peer_id];
                } else if (type == "offer") {

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

    NodeManager::NodeManager(
            PeerType peer_type
    ) : peer_state(PeerState::Closed),
        full_peer_count(0),
        light_peer_count(0) {

        std::promise<int> barrier;
        auto future = barrier.get_future();
        ws.setUrl("ws://127.0.0.1:8080");
        ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr &message) {
            if (message->type == ix::WebSocketMessageType::Open) {

                ws.send(SignalingMessage{SignalingMessageType::IdentityRequest});
            } else if (message->type == ix::WebSocketMessageType::Message) {

                auto msg_json = nlohmann::json::parse(message->str);
                auto msg = SignalingMessage::from_json(msg_json);

                if (msg.type == SignalingMessageType::IdentityResponse) {

                    barrier.set_value(msg.content.get<int>());
                } else {

                    on_signaling_message(msg);
                }
            }
        });
        ws.start();
        spdlog::info("NodeManager: Waiting for identity from signaling server...");
        my_id = future.get();
        spdlog::info("NodeManager: Acquired identity {}", my_id);

        m_dispatcher.appendListener(
                PeerMessageType::PeerTypeRequest,
                [this, peer_type](const PeerMessage &message) {

                    send(
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
        m_dispatcher.appendListener(
                PeerMessageType::PeerStateUpdate,
                [this](const PeerMessage &message) {
                    auto new_state = message.content().get<PeerState>();
                    std::lock_guard l(map_mutex);
                    if (peer_state_map.contains(message.peer_id())) {
                        spdlog::info(
                                "NodeManager: Updating State of {} to {}", message.peer_id(),
                                to_string(new_state)
                        );
                        peer_state_map[message.peer_id()] = new_state;
                    }
                }
        );
    }

    void NodeManager::wait() {

        std::unique_lock l(blocking_mutex);
        blocking_cv.wait(l);
    }

    void NodeManager::append_listener(PeerMessageType type, const std::function<void(PeerMessage)> &listener) {

        m_dispatcher.appendListener(type, listener);
    }

    int NodeManager::id() const {

        return my_id;
    }

    void NodeManager::add_peer_connection(
            int id,
            std::shared_ptr<rtc::PeerConnection> peer_connection
    ) {

        peer_connection->onStateChange(
                [id, self = weak_from_this()](rtc::PeerConnection::State state) {

                    switch (state) {
                        case rtc::PeerConnection::State::Closed:
                        case rtc::PeerConnection::State::Disconnected: {
                            if (auto ptr = self.lock()) {

                                ptr->peer_map.erase(id);
                                ptr->channel_map.erase(id);
                                ptr->peer_type_map.erase(id);
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
        );

        std::lock_guard l(map_mutex);
        peer_map.emplace(id, std::move(peer_connection));
    }

    void NodeManager::add_channel(
            int id,
            std::shared_ptr<rtc::DataChannel> channel
    ) {

        auto krapi_channel = std::make_shared<KrapiRTCDataChannel>(std::move(channel));
        krapi_channel->append_listener(
                KrapiRTCDataChannel::Event::Message,
                [this](const PeerMessage &message) {
                    m_dispatcher.dispatch(message.type(), message);
                }
        );

        krapi_channel->append_listener(
                KrapiRTCDataChannel::Event::Open,
                [id, wthis = weak_from_this()]() {
                    auto self = wthis.lock();
                    if (!self)
                        return;

                    auto resp1 = self->send(
                            id,
                            PeerMessage{
                                    PeerMessageType::PeerTypeRequest,
                                    self->my_id,
                                    PeerMessage::create_tag()
                            }
                    ).get();
                    auto peer_type = resp1.content().get<PeerType>();
                    self->peer_type_map[id] = peer_type;

                    if (peer_type == PeerType::Full) {

                        self->full_peer_count++;
                    } else {

                        self->light_peer_count++;
                    }

                    auto state = self->request_peer_state(id);
                    self->peer_threshold_cv.notify_one();
                    spdlog::info("NodeManager: Initial State of {} is {}", id, to_string(state));
                }
        );
        krapi_channel->append_listener(
                KrapiRTCDataChannel::Event::Close,
                [id, this]() {

                    if (peer_type_map[id] == PeerType::Full) {

                        full_peer_count--;
                    } else {

                        light_peer_count--;
                    }
                }
        );
        std::lock_guard l(map_mutex);
        channel_map.emplace(id, std::move(krapi_channel));
    }

    void NodeManager::broadcast(
            const PeerMessage &message,
            const std::optional<PeerMessageCallback> &callback,
            bool include_light_nodes
    ) {

        for (auto &[id, peer_channel]: channel_map) {

            if (include_light_nodes) {

                peer_channel->send(message, callback);
            } else {

                if (peer_type_map[id] == PeerType::Full) {

                    peer_channel->send(message, callback);
                }
            }
        }
    }

    std::future<PeerMessage> NodeManager::send(
            int id,
            PeerMessage message,
            std::optional<PeerMessageCallback> callback
    ) {

        std::shared_ptr<KrapiRTCDataChannel> channel;

        {
            std::lock_guard l(map_mutex);
            channel = channel_map.find(id)->second;
        }
        if (channel != nullptr) {

            return channel->send(std::move(message), std::move(callback));
        }
        return {};
    }

    std::vector<std::shared_ptr<KrapiRTCDataChannel>> NodeManager::get_channels() {

        std::vector<std::shared_ptr<KrapiRTCDataChannel>> ans;
        for (auto [id, channel]: channel_map) {
            ans.push_back(std::move(channel));
        }
        return ans;
    }

    std::vector<int> NodeManager::peer_ids_of_type(PeerType type) {

        std::vector<int> ans;
        for (const auto &[id, tp]: peer_type_map) {
            if (tp == type)
                ans.push_back(id);
        }
        return ans;
    }

    void NodeManager::wait_for(PeerType peer_type, int peer_count) {

        std::unique_lock l(peer_threshold_mutex);
        if (peer_type == PeerType::Full) {

            peer_threshold_cv.wait(l, [peer_count, this]() {

                return full_peer_count >= peer_count;
            });
        } else {

            peer_threshold_cv.wait(l, [peer_count, this]() {

                return light_peer_count >= peer_count;
            });
        }
    }

    NodeManager::~NodeManager() {

        spdlog::error("NodeManagerByeBye :\'(");
    }

    PeerState NodeManager::request_peer_state(int id) {

        if (peer_state_map.contains(id)) {

            return peer_state_map[id];
        } else {

            auto resp = send(
                    id,
                    PeerMessage{
                            PeerMessageType::PeerStateRequest,
                            my_id,
                            PeerMessage::create_tag()
                    }
            ).get();
            auto state = resp.content().get<PeerState>();

            {
                std::lock_guard l(map_mutex);
                peer_state_map[id] = state;
            }
            return state;
        }
    }

    PeerState NodeManager::get_state() const {

        return peer_state;
    }

    void NodeManager::update_state_to(PeerState new_state) {

        if (new_state != peer_state) {
            {
                std::lock_guard l(peer_state_mutex);
                peer_state = new_state;
                spdlog::info("NodeManager: Updated Local State to {}", to_string(new_state));
                broadcast(
                        PeerMessage{
                                PeerMessageType::PeerStateUpdate,
                                my_id,
                                PeerMessage::create_tag(),
                                new_state
                        },
                        std::nullopt,
                        true
                );
            }
        }
    }
} // krapi