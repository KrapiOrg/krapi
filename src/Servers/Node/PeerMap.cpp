//
// Created by mythi on 12/11/22.
//

#include "PeerMap.h"

namespace krapi {
    void PeerMap::add_peer(int id, std::shared_ptr<rtc::PeerConnection> peer_connection) {

        std::lock_guard l(mutex);
        peer_map.emplace(id, std::move(peer_connection));
    }

    void PeerMap::add_channel(int id, std::shared_ptr<rtc::DataChannel> channel) {

        std::lock_guard l(mutex);
        channel_map.emplace(id, std::move(channel));
    }

    bool PeerMap::contains_peer(int id) {

        return peer_map.contains(id);
    }

    std::shared_ptr<rtc::PeerConnection> PeerMap::get_peer(int id) {

        return peer_map[id];
    }

    void PeerMap::send_to(int id) {

    }

    void PeerMap::broadcast(std::string message) {

        for (auto &[id, channel]: channel_map) {

            if (channel->isOpen()) {

                channel->send(message);
            }
        }
    }

} // krapi