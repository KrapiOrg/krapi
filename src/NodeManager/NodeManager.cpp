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
                            [=, this]() {
                                {
                                    std::lock_guard l(m_promise_map_mutex);
                                    promise_map->emplace(id, std::make_shared<PromiseMap>());
                                }
                                {
                                    std::lock_guard l(m_future_map_mutex);
                                    future_map->emplace(id, std::make_shared<FutureMap>());
                                }
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

                                {
                                    std::lock_guard l(m_promise_map_mutex);
                                    if (promise_map->at(id)->contains(peer_message.tag()))
                                        promise_map->at(id)->at(peer_message.tag())->set_value(peer_message);
                                }

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
        std::lock_guard l(m_channel_map_mutex);
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

                                std::lock_guard l(ptr->m_connection_map_mutex);
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

        std::lock_guard l(m_connection_map_mutex);
        m_connection_map.emplace(id, pc);
        return pc;
    }

    NodeManager::NodeManager(
            PeerType pt
    ) : m_peer_state(PeerState::Closed),
        my_id(-1),
        m_peer_type(pt),
        m_peer_count(0),
        promise_map(std::make_shared<PerPeerPromiseMap>()),
        future_map(std::make_shared<PerPeerFutureMap>()) {

        spdlog::info("NodeManager: Acquiring identity...");
        my_id = m_signaling_client.get_identity();
        spdlog::info("NodeManager: Acquired identity {}", my_id);

        m_signaling_client.on_rtc_setup(
                [this](SignalingMessage msg) {

                    auto peer_id = msg.content["id"].get<int>();
                    auto type = msg.content["type"].get<std::string>();

                    std::shared_ptr<rtc::PeerConnection> pc;

                    auto safe_connection_map_access = [&pc, &peer_id, this]() {

                        std::lock_guard l(m_connection_map_mutex);
                        if (m_connection_map.contains(peer_id)) {

                            pc = m_connection_map[peer_id];
                            return true;
                        }
                        return false;
                    };

                    if (safe_connection_map_access());
                    else if (type == "offer") {

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
                    std::lock_guard l(m_peer_states_map_mutex);
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

    ErrorOr<NodeManager::Future> NodeManager::send(
            int id,
            const PeerMessage &message
    ) {


        return m_send_queue.submit(
                [=, this]() -> ErrorOr<NodeManager::Future> {

                    {
                        std::lock_guard l(m_channel_map_mutex);
                        if (!m_channel_map.contains(id))
                            return tl::make_unexpected(
                                    KrapiErr{
                                            fmt::format("Channel {} was closed", id)
                                    }
                            );

                        m_channel_map.find(id)->second->send(message.to_json().dump());
                    }
                    auto promise = std::make_shared<Promise>();
                    auto future = promise->get_future().share();
                    {
                        std::lock_guard l(m_promise_map_mutex);
                        if (!promise_map->contains(id))
                            return tl::make_unexpected(
                                    KrapiErr{
                                            fmt::format("PromiseMap does not have an entry for {}", id)
                                    }
                            );
                        promise_map->find(id)->second->emplace(message.tag(), std::move(promise));
                    }
                    {
                        std::lock_guard l(m_future_map_mutex);
                        if (!future_map->contains(id))
                            return tl::make_unexpected(
                                    KrapiErr{
                                            fmt::format("PromiseMap does not have an entry for {}", id)
                                    }
                            );
                        future_map->find(id)->second->emplace(message.tag(), future);
                    }
                    return future;
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

    ErrorOr<PeerState> NodeManager::request_peer_state(int id) {

        {
            std::lock_guard l(m_peer_states_map_mutex);
            if (m_peer_states_map.contains(id)) [[likely]] {

                return m_peer_states_map[id];
            }
        }

        auto resp =
                TRY(
                        send(
                                id,
                                PeerMessage{
                                        PeerMessageType::PeerStateRequest,
                                        my_id,
                                        PeerMessage::create_tag()
                                }
                        )
                ).get();

        if (!resp.has_value())
            return tl::unexpected(resp.error());

        std::lock_guard l(m_peer_states_map_mutex);
        m_peer_states_map[id] = resp.value().content().get<PeerState>();
        return m_peer_states_map[id];
    }

    PeerState NodeManager::get_state() const {

        std::lock_guard l(m_peer_state_mutex);
        return m_peer_state;
    }

    MultiFuture<NodeManager::PromiseType> NodeManager::broadcast(
            PeerMessage message,
            const std::set<PeerType> &excluded_types,
            const std::set<PeerState> &excluded_states
    ) {

        MultiFuture<NodeManager::PromiseType> futures;
        std::unordered_map<int, std::shared_ptr<rtc::DataChannel>> channels;
        {
            std::lock_guard l(m_channel_map_mutex);
            channels = m_channel_map;
        }
        for (auto [id, channel]: channels) {

            if(!channel)
                continue;
            auto type = request_peer_type(id);
            auto state = request_peer_state(id);
            if (!type.has_value() || !state.has_value())
                continue;

            if (excluded_types.contains(type.value()))
                continue;
            if (excluded_states.contains(state.value()))
                continue;

            message.randomize_tag();
            auto send_future = send(id, message);
            if (!send_future.has_value())
                continue;

            futures.push_back(send_future.value());
        }
        return futures;
    }

    void NodeManager::on_channel_close(int id) {

        spdlog::warn("DataChannel with {} has closed", id);
        {
            std::lock_guard l(m_peer_states_map_mutex);
            m_peer_states_map.erase(id);
        }
        {
            std::lock_guard l(m_peer_types_map_mutex);
            m_peer_types_map.erase(id);
        }
        auto is_ready = [](const std::shared_future<PromiseType> &f) {

            return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        };
        {
            std::lock_guard l(m_promise_map_mutex);
            for (auto [message_id, promise]: *promise_map->at(id)) {
                if(!promise)
                    continue;
                {
                    std::lock_guard l2(m_future_map_mutex);
                    auto future = future_map->at(id)->at(message_id);
                    if (!is_ready(future)) {
                        promise->set_value(tl::make_unexpected(KrapiErr{"Channel Closed"}));

                    }
                }
            }
        }

        m_peer_count--;
        std::lock_guard l(m_channel_map_mutex);
        m_channel_map.erase(id);
    }

    ErrorOr<PeerType> NodeManager::request_peer_type(int id) {

        {
            std::lock_guard l(m_peer_types_map_mutex);
            if (m_peer_types_map.contains(id)) [[likely]] {

                return m_peer_types_map[id];
            }
        }

        auto resp = TRY(
                send(
                        id,
                        PeerMessage{
                                PeerMessageType::PeerTypeRequest,
                                my_id,
                                PeerMessage::create_tag()
                        }
                )
        ).get();

        if (!resp.has_value())
            return tl::unexpected(resp.error());

        std::lock_guard l(m_peer_types_map_mutex);
        m_peer_types_map[id] = resp.value().content().get<PeerType>();
        return m_peer_types_map[id];

    }

    void NodeManager::set_state(PeerState new_state) {

        {
            std::lock_guard l(m_peer_state_mutex);
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

    ErrorOr<std::vector<std::tuple<int, PeerType, PeerState>>> NodeManager::
    get_peers(
            const std::set<PeerType> &types,
            const std::set<PeerState> &states
    ) {

        auto ans = std::vector<std::tuple<int, PeerType, PeerState>>{};
        std::unordered_map<int, std::shared_ptr<rtc::DataChannel>> channels;
        {
            std::lock_guard l(m_channel_map_mutex);
            channels = m_channel_map;
        }
        for (auto [id, channel]: channels) {
            if(!channel)
                continue;
            auto pt = TRY(request_peer_type(id));
            auto state = TRY(request_peer_state(id));

            if (types.contains(pt) && states.contains(state)) {
                ans.emplace_back(id, pt, state);
            }
        }
        return ans;

    }

} // krapi