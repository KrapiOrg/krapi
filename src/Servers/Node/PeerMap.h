//
// Created by mythi on 12/11/22.
//

#ifndef NODE_PEERMAP_H
#define NODE_PEERMAP_H

#include <unordered_map>
#include <mutex>
#include <memory>
#include "rtc/peerconnection.hpp"
#include "PeerMessage.h"
#include "PeerType.h"

namespace krapi {

    class PeerMap {
        std::unordered_map<int, std::shared_ptr<rtc::PeerConnection>> peer_map;
        std::unordered_map<int, PeerType> peer_type_map;
        std::unordered_map<int, std::shared_ptr<rtc::DataChannel>> channel_map;
        std::mutex mutex;

    public:

        void add_peer(int id, std::shared_ptr<rtc::PeerConnection> peer_connection);

        void add_channel(int id, std::shared_ptr<rtc::DataChannel> channel);

        void set_peer_type(int id, PeerType type);
        PeerType get_peer_type(int id);

        bool contains_peer(int id);

        std::shared_ptr<rtc::PeerConnection> get_peer(int id);
        std::shared_ptr<rtc::DataChannel> get_channel(int id);

        void broadcast(PeerMessage message, int my_id);
    };


} // krapi

#endif //NODE_PEERMAP_H
