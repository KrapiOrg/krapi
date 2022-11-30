//
// Created by mythi on 24/11/22.
//

#include "NodeManager.h"
#include "spdlog/spdlog.h"
#include "ErrorOr.h"

using namespace std::chrono_literals;

namespace krapi {
    std::shared_ptr<rtc::PeerConnection> NodeManager::create_connection(int id) {

        auto pc = std::make_shared<rtc::PeerConnection>(m_rtc_config);

        pc->onStateChange(
                [=, self = weak_from_this()](rtc::PeerConnection::State state) {
                    switch (state) {
                        case rtc::PeerConnection::State::Closed:
                        case rtc::PeerConnection::State::Disconnected: {
                            if (auto ptr = self.lock()) {

                                ptr->connection_map.erase(id);
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
        );

        pc->onLocalDescription([=, this](const rtc::Description &description) {
            auto desc = nlohmann::json{
                    {"id",          id},
                    {"type",        description.typeString()},
                    {"description", std::string(description)}
            };
            auto msg = SignalingMessage{SignalingMessageType::RTCSetup, desc};
            m_signaling_socket.send(msg);
        });

        pc->onLocalCandidate([=, this](const rtc::Candidate &candidate) {
            auto cand = nlohmann::json{
                    {"id",        id},
                    {"type",      "candidate"},
                    {"candidate", std::string(candidate)},
                    {"mid",       candidate.mid()}
            };
            auto msg = SignalingMessage{SignalingMessageType::RTCSetup, cand};
            m_signaling_socket.send(msg);
        });

        pc->onDataChannel(
                [=, this](
                        std::shared_ptr<rtc::DataChannel> channel
                ) {
                    channel->onOpen([this]() {
                        m_peer_handler_queue.push_task(
                                [this]() {
                                    m_peer_count++;
                                    m_peer_count.notify_all();
                                }
                        );
                    });
                    channel->onMessage(
                            [this](rtc::message_variant rtc_message) {
                                auto msg_str = std::get<std::string>(rtc_message);
                                auto msg_json = nlohmann::json::parse(msg_str);
                                auto peer_message = PeerMessage::from_json(msg_json);
                                m_receive_queue.push_task(
                                        [=, this]() {
                                            promise_map[peer_message.tag()].set_value(peer_message);
                                            m_dispatcher.dispatch(peer_message.type(), peer_message);
                                        }
                                );
                            }
                    );

                    channel->onClosed(
                            [id, this]() {
                                m_peer_handler_queue.push_task(
                                        [=, this]() {
                                            m_peer_count--;
                                            on_channel_close(id);
                                        }
                                );
                            }
                    );
                    channel_map.emplace(id, channel);
                });

        connection_map.emplace(id, pc);
        return pc;
    }

    void NodeManager::on_signaling_message(const SignalingMessage &rsp) {

        switch (rsp.type) {

            case SignalingMessageType::PeerAvailable: {
                auto id = rsp.content.get<int>();
                auto pc = create_connection(id);
                auto dc = pc->createDataChannel("krapi");

                dc->onOpen(
                        [this]() {
                            m_peer_handler_queue.push_task(
                                    [this]() {
                                        m_peer_count++;
                                        m_peer_count.notify_all();
                                    }
                            );
                        }
                );

                dc->onMessage(
                        [this](rtc::message_variant rtc_message) {
                            auto msg_str = std::get<std::string>(rtc_message);
                            auto msg_json = nlohmann::json::parse(msg_str);
                            auto peer_message = PeerMessage::from_json(msg_json);
                            m_receive_queue.push_task(
                                    [=, this]() {

                                        promise_map[peer_message.tag()].set_value(peer_message);
                                        m_dispatcher.dispatch(peer_message.type(), peer_message);
                                    }
                            );
                        }
                );

                dc->onClosed(
                        [id, this]() {
                            m_peer_handler_queue.push_task(
                                    [id, this]() {
                                        on_channel_close(id);
                                    }
                            );

                        }
                );

                channel_map.emplace(id, dc);
            }
                break;
            case SignalingMessageType::RTCSetup: {

                auto peer_id = rsp.content["id"].get<int>();
                auto type = rsp.content["type"].get<std::string>();

                std::shared_ptr<rtc::PeerConnection> pc;
                if (connection_map.contains(peer_id)) {

                    pc = connection_map[peer_id];
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
            PeerType pt
    ) : m_peer_state(PeerState::Closed),
        m_peer_type(pt),
        m_peer_count(0) {

        std::promise<int> barrier;
        auto future = barrier.get_future();
        m_signaling_socket.setUrl("signaling://127.0.0.1:8080");
        m_signaling_socket.setOnMessageCallback([&](const ix::WebSocketMessagePtr &message) {
            if (message->type == ix::WebSocketMessageType::Open) {

                m_signaling_socket.send(SignalingMessage{SignalingMessageType::IdentityRequest});
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
        m_signaling_socket.start();
        my_id = future.get();

        m_dispatcher.appendListener(
                PeerMessageType::PeerTypeRequest,
                [this](const PeerMessage &message) {

                    (void) send(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::PeerTypeResponse,
                                    my_id,
                                    message.tag(),
                                    m_peer_type
                            }
                    );
                }
        );
        m_dispatcher.appendListener(
                PeerMessageType::PeerStateRequest,
                [this](const PeerMessage &message) {

                    PeerState my_state;
                    {
                        std::lock_guard l(m_peer_state_mutex);
                        my_state = m_peer_state;
                    }

                    (void) send(
                            message.peer_id(),
                            PeerMessage{
                                    PeerMessageType::PeerStateResponse,
                                    my_id,
                                    message.tag(),
                                    my_state
                            }
                    );
                }
        );
    }

    void NodeManager::wait() {

        std::unique_lock l(m_blocking_mutex);
        m_blocking_cv.wait(l);
    }

    void NodeManager::append_listener(
            PeerMessageType type,
            const std::function<void(PeerMessage)> &listener
    ) {

        m_dispatcher.appendListener(type, listener);
    }

    int NodeManager::id() const {

        return my_id;
    }

    std::future<PeerMessage> NodeManager::send(
            int id,
            const PeerMessage &message
    ) {


        return m_send_queue.submit(
                [=, this]() -> std::future<PeerMessage> {

                    channel_map[id]->send(message.to_json().dump());
                    return promise_map[message.tag()].get_future();
                }
        ).get();
    }


    void NodeManager::wait_for(PeerType pt, int numbers_of_peers) {

        m_peer_count.wait(0);
        while (true) {
            auto ids = peer_ids_of_type(pt);
            if (ids.size() >= numbers_of_peers) {
                break;
            }
        }
    }

    PeerState NodeManager::request_peer_state(int id) {

        auto resp = send(
                id,
                PeerMessage{
                        PeerMessageType::PeerStateRequest,
                        my_id,
                        PeerMessage::create_tag()
                }
        ).get();
        return resp.content().get<PeerState>();
    }

    PeerState NodeManager::get_state() const {

        std::lock_guard l(m_peer_state_mutex);
        return m_peer_state;
    }

    MultiFuture<PeerMessage> NodeManager::broadcast(
            const PeerMessage &message,
            bool include_light_nodes
    ) {

        MultiFuture<PeerMessage> futures;
        for (const auto &[id, channel]: channel_map) {
            auto state = request_peer_state(id);
            auto type = request_peer_type(id);

            if (state != PeerState::Open)
                continue;

            if (type == PeerType::Light && !include_light_nodes)
                continue;

            futures.push_back(send(id, message));

        }
        return futures;
    }

    void NodeManager::on_channel_close(int id) {

        channel_map.erase(id);
    }

    PeerType NodeManager::request_peer_type(int id) {

        auto resp = send(
                id,
                PeerMessage{
                        PeerMessageType::PeerTypeRequest,
                        my_id,
                        PeerMessage::create_tag()
                }
        ).get();
        return resp.content().get<PeerType>();
    }

    void NodeManager::set_state(PeerState new_state) {

        std::lock_guard l(m_peer_state_mutex);
        m_peer_state = new_state;
    }

    std::vector<int> NodeManager::peer_ids_of_type(PeerType type) {
        auto ans = std::vector<int>{};
        for(const auto &[id,_] : channel_map) {
            auto pt = request_peer_type(id);
            if(pt == type)
                ans.push_back(id);
        }
        return ans;
    }

} // krapi