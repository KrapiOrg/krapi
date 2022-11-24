//
// Created by mythi on 12/11/22.
//

#include "PeerMap.h"


namespace krapi {
    void PeerMap::add_peer(int id, std::shared_ptr<rtc::PeerConnection> peer_connection) {

        std::lock_guard l(mutex);
        peer_connection->onStateChange([id, this](rtc::PeerConnection::State state) {
            if (state == rtc::PeerConnection::State::Closed) {
                peer_map.erase(id);
            }
        });
        peer_map.emplace(id, std::move(peer_connection));
    }

    void PeerMap::add_channel(
            int id,
            std::shared_ptr<rtc::DataChannel> channel,
            std::optional<PeerMessageCallback> callback
    ) {

        std::lock_guard l(mutex);

        channel_map.emplace(
                id,
                std::make_shared<KrapiRTCDataChannel>(
                        std::move(channel),
                        [id, this]() {
                            channel_map.erase(id);
                        },
                        std::move(callback)
                )
        );
    }

    bool PeerMap::contains_peer(int id) {

        return peer_map.contains(id);
    }

    std::shared_ptr<rtc::PeerConnection> PeerMap::get_peer(int id) {

        return peer_map[id];
    }

    void PeerMap::broadcast(
            PeerMessage message,
            std::optional<PeerMessageCallback> callback,
            bool include_light_nodes
    ) {

        auto cm = std::unordered_map<int, std::shared_ptr<KrapiRTCDataChannel>>{};

        {
            std::lock_guard l(mutex);
            cm = channel_map;
        }

        for (auto &[id, peer_channel]: cm) {

            if (include_light_nodes) {

                peer_channel->send(message, callback);
            } else {

                auto resp = peer_channel->send(
                        PeerMessage{
                                PeerMessageType::PeerTypeRequest,
                                message.peer_id(),
                                PeerMessage::create_tag()
                        }
                ).get();

                if (resp.type() == PeerMessageType::PeerTypeResponse) {

                    auto peer_type = resp.content().get<PeerType>();
                    if (peer_type != PeerType::Light) {

                        if (peer_channel->is_open()) {

                            peer_channel->send(message);
                        }

                    }
                }
            }


        }
    }

    std::shared_future<PeerMessage> PeerMap::send_message(
            int id,
            PeerMessage message,
            std::optional<PeerMessageCallback> callback
    ) {


        std::lock_guard l(mutex);
        if (channel_map.contains(id)) {

            auto dc = channel_map[id];
            return dc->send(std::move(message), std::move(callback));
        }
        return {};
    }

    std::vector<std::shared_ptr<KrapiRTCDataChannel>> PeerMap::get_channels() {

        std::lock_guard l(mutex);
        std::vector<std::shared_ptr<KrapiRTCDataChannel>> ans;
        for (auto [id, channel]: channel_map) {
            ans.push_back(std::move(channel));
        }
        return ans;
    }

} // krapi