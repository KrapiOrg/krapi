//
// Created by mythi on 24/11/22.
//

#include "NodeManager.h"
#include "spdlog/spdlog.h"
#include "ErrorOr.h"

using namespace std::chrono_literals;

namespace krapi {
    std::future<int> NodeManager::set_up_datachannel(int id, std::shared_ptr<rtc::DataChannel> channel) {

        auto open_promise = std::make_shared<std::promise<int>>();
        channel->onOpen(
                [=, this]() {
                    m_peer_handler_queue.push_task(
                            [=]() {
                                open_promise->set_value(id);
                            }
                    );
                }
        );
        channel->onMessage(
                [id, this](rtc::message_variant rtc_message) {
                    m_receive_queue.push_task(
                            [=, this]() {
                                auto msg_str = std::get<std::string>(rtc_message);
                                auto msg_json = nlohmann::json::parse(msg_str);
                                auto peer_message = PeerMessage::from_json(msg_json);
                                promise_map[id][peer_message.tag()].set_value(peer_message);
                                m_dispatcher.dispatch(peer_message.type(), peer_message);
                            }
                    );
                }
        );

        channel->onClosed(
                [id, this]() {
                    m_peer_handler_queue.push_task(
                            [=, this]() {
                                on_channel_close(id);
                            }
                    );
                }
        );
        m_channel_map.emplace(id, channel);
        return open_promise->get_future();
    }

    std::shared_ptr<rtc::PeerConnection> NodeManager::create_connection(int id) {

        auto pc = std::make_shared<rtc::PeerConnection>(m_rtc_config);

        pc->onStateChange(
                [=, self = weak_from_this()](rtc::PeerConnection::State state) {
                    switch (state) {
                        case rtc::PeerConnection::State::Closed:
                        case rtc::PeerConnection::State::Disconnected: {
                            if (auto ptr = self.lock()) {

                                ptr->m_connection_map.erase(id);
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
            (void) m_signaling_client.send(SignalingMessage{SignalingMessageType::RTCSetup, desc});
        });

        pc->onLocalCandidate([=, this](const rtc::Candidate &candidate) {
            auto cand = nlohmann::json{
                    {"id",        id},
                    {"type",      "candidate"},
                    {"candidate", std::string(candidate)},
                    {"mid",       candidate.mid()}
            };
            (void) m_signaling_client.send(SignalingMessage{SignalingMessageType::RTCSetup, cand});
        });

        pc->onDataChannel(
                [=, this](auto &&channel) {
                    (void) set_up_datachannel(id, std::forward<decltype(channel)>(channel));
                }
        );

        m_connection_map.emplace(id, pc);
        return pc;
    }

    NodeManager::NodeManager(
            PeerType pt
    ) : m_peer_state(PeerState::Closed),
        my_id(-1),
        m_peer_type(pt),
        m_peer_count(0) {

        spdlog::info("NodeManager: Acquiring identity...");
        my_id = m_signaling_client.get_identity();
        spdlog::info("NodeManager: Acquired identity {}", my_id);

        m_signaling_client.on_rtc_setup(
                [this](SignalingMessage msg) {

                    auto peer_id = msg.content["id"].get<int>();
                    auto type = msg.content["type"].get<std::string>();

                    std::shared_ptr<rtc::PeerConnection> pc;
                    if (m_connection_map.contains(peer_id)) {

                        pc = m_connection_map[peer_id];
                    } else if (type == "offer") {

                        pc = create_connection(peer_id);
                    } else {

                        return;
                    }

                    if (type == "offer" || type == "answer") {

                        auto sdp = msg.content["description"].get<std::string>();
                        pc->setRemoteDescription(rtc::Description(sdp, type));
                    } else if (type == "candidate") {

                        auto sdp = msg.content["candidate"].get<std::string>();
                        auto mid = msg.content["mid"].get<std::string>();
                        pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
                    }
                }
        );

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
        m_dispatcher.appendListener(
                PeerMessageType::PeerStateUpdate,
                [this](const PeerMessage &message) {
                    auto state = message.content().get<PeerState>();
                    spdlog::info("Updating State of {} to {}", message.peer_id(), to_string(state));
                    m_peer_states_map[message.peer_id()] = state;
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

                    m_channel_map[id]->send(message.to_json().dump());
                    return promise_map[id][message.tag()].get_future();
                }
        ).get();
    }


    MultiFuture<int> NodeManager::connect_to_peers() {

        auto avail_resp = m_signaling_client.send(SignalingMessageType::AvailablePeersRequest).get();
        auto avail_peers = avail_resp.content.get<std::vector<int>>();

        MultiFuture<int> futures;
        for (auto id: avail_peers) {

            auto connection = create_connection(id);
            auto channel = connection->createDataChannel("krapi");
            futures.push_back(set_up_datachannel(id, std::move(channel)));
        }

        return futures;
    }

    PeerState NodeManager::request_peer_state(int id) {

        if (m_peer_states_map.contains(id)) {

            return m_peer_states_map[id];
        } else {

            auto resp = send(
                    id,
                    PeerMessage{
                            PeerMessageType::PeerStateRequest,
                            my_id,
                            PeerMessage::create_tag()
                    }
            ).get();
            m_peer_states_map[id] = resp.content().get<PeerState>();
            return m_peer_states_map[id];
        }

    }

    PeerState NodeManager::get_state() const {

        std::lock_guard l(m_peer_state_mutex);
        return m_peer_state;
    }

    MultiFuture<PeerMessage> NodeManager::broadcast(
            PeerMessage message,
            const std::set<PeerType> &excluded_types,
            const std::set<PeerState> &excluded_states
    ) {

        MultiFuture<PeerMessage> futures;
        for (const auto &[id, channel]: m_channel_map) {

            auto type = request_peer_type(id);
            auto state = request_peer_state(id);

            if (excluded_types.contains(type))
                continue;
            if (excluded_states.contains(state))
                continue;

            message.randomize_tag();
            futures.push_back(send(id, message));
        }
        return futures;
    }

    void NodeManager::on_channel_close(int id) {

        spdlog::warn("DataChannel with {} has closed", id);
        m_peer_states_map.erase(id);
        m_peer_types_map.erase(id);
        promise_map.erase(id);

        m_peer_count--;
        m_channel_map.erase(id);
    }

    PeerType NodeManager::request_peer_type(int id) {

        if (m_peer_types_map.contains(id)) {

            return m_peer_types_map[id];
        } else {

            auto resp = send(
                    id,
                    PeerMessage{
                            PeerMessageType::PeerTypeRequest,
                            my_id,
                            PeerMessage::create_tag()
                    }
            ).get();
            m_peer_types_map[id] = resp.content().get<PeerType>();
            return m_peer_types_map[id];
        }
    }

    void NodeManager::set_state(PeerState new_state) {

        {
            std::lock_guard l(m_peer_state_mutex);
            if (m_peer_state == new_state)
                return;
            m_peer_state = new_state;
        }
        (void) broadcast(
                PeerMessage{
                        PeerMessageType::PeerStateUpdate,
                        my_id,
                        m_peer_state
                }
        );
    }

    std::vector<int> NodeManager::peer_ids_of_type(PeerType type) {

        auto ans = std::vector<int>{};
        for (const auto &[id, _]: m_channel_map) {
            auto state = request_peer_state(id);
            auto pt = request_peer_type(id);
            if (pt == type) {

                if (state != PeerState::Closed && state != PeerState::Error)
                    ans.push_back(id);
                else
                    spdlog::warn("{} is of type {} but is {}", id, to_string(type), to_string(state));
            }
        }
        return ans;
    }

} // krapi