//
// Created by mythi on 12/11/22.
//

#ifndef SHARED_MODELS_PEERMAP_H
#define SHARED_MODELS_PEERMAP_H

#include <unordered_map>
#include <mutex>
#include <memory>
#include "rtc/peerconnection.hpp"

namespace krapi {

    class PeerMap {
        std::unordered_map<int, std::shared_ptr<rtc::PeerConnection>> peer_map;
        std::unordered_map<int, std::shared_ptr<rtc::DataChannel>> channel_map;
        std::mutex mutex;

    public:

        void add_peer(int id, std::shared_ptr<rtc::PeerConnection> peer_connection);

        void add_channel(int id, std::shared_ptr<rtc::DataChannel> channel);

        bool contains_peer(int id);

        std::shared_ptr<rtc::PeerConnection> get_peer(int id);

        void send_to(int id);

        void broadcast(std::string message);
    };



} // krapi

#endif //SHARED_MODELS_PEERMAP_H
