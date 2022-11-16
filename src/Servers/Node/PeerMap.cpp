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

    void PeerMap::broadcast(PeerMessage message, int my_id) {
        auto old_ignore_list = message.ignore_list;
        auto new_ignore_list = message.ignore_list;

        new_ignore_list.insert(my_id);

        for (auto &[id, channel]: channel_map) {
            new_ignore_list.insert(id);
        }
        message.ignore_list = new_ignore_list;

        auto message_str = message.to_string();
        for (auto &[id, channel]: channel_map) {
            if (!old_ignore_list.contains(id)) {

                if (channel->isOpen()) {

                    channel->send(message_str);
                }
            }
        }
    }

} // krapi