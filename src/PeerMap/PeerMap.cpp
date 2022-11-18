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

    std::shared_ptr<rtc::DataChannel> PeerMap::get_channel(int id) {

        return channel_map[id];
    }

    void PeerMap::broadcast(PeerMessage message, int my_id) {

        auto old_ignore_list = message.ignore_list;
        auto new_ignore_list = message.ignore_list;

        new_ignore_list.insert(my_id);

        for (auto &[id, channel]: channel_map) {
            new_ignore_list.insert(id);
        }
        message.ignore_list = new_ignore_list;
        auto cm = std::unordered_map<int, std::shared_ptr<rtc::DataChannel>>{};
        auto ct = std::unordered_map<int, PeerType>{};
        {
            std::lock_guard l(mutex);
            cm = channel_map;
            ct = peer_type_map;
        }
        auto message_str = message.to_string();
        for (auto &[id, channel]: cm) {

            if (ct.contains(id) && ct[id] == PeerType::Light)
                continue;

            if (!old_ignore_list.contains(id)) {

                if (channel->isOpen()) {

                    channel->send(message_str);
                }
            }
        }
    }

    void PeerMap::set_peer_type(int id, PeerType type) {

        {
            std::lock_guard l(mutex);
            peer_type_map[id] = type;
        }
        m_dispatcher.dispatch(Event::PeerTypeKnown, id);
    }

    PeerType PeerMap::get_peer_type(int id) {

        std::lock_guard l(mutex);
        return peer_type_map[id];
    }

    void PeerMap::append_listener(PeerMap::Event event, std::function<void(int)> callback) {

        m_dispatcher.appendListener(event, callback);
    }

    std::vector<int> PeerMap::get_light_node_ids() {

        auto ptm = std::unordered_map<int, PeerType>{};
        auto cm = std::unordered_map<int, std::shared_ptr<rtc::DataChannel>>{};

        {
            std::lock_guard l(mutex);
            ptm = peer_type_map;
            cm = channel_map;
        }
        auto ans = std::vector<int>{};
        for (auto &[id, channel]: cm) {
            if (channel->isOpen()) {
                if (ptm.contains(id) && ptm[id] == PeerType::Light) {
                    ans.push_back(id);
                }
            }
        }
        return ans;
    }

} // krapi