//
// Created by mythi on 12/11/22.
//

#pragma once

#include <unordered_map>
#include <mutex>
#include <memory>
#include "rtc/peerconnection.hpp"
#include "PeerMessage.h"
#include "PeerType.h"
#include "eventpp/eventdispatcher.h"
#include "KrapiRTCDataChannel.h"

namespace krapi {

    class PeerMap {

    private:

        std::unordered_map<int, std::shared_ptr<rtc::PeerConnection>> peer_map;
        std::unordered_map<int, std::shared_ptr<KrapiRTCDataChannel>> channel_map;
        std::mutex mutex;


    public:

        void add_peer(int id, std::shared_ptr<rtc::PeerConnection> peer_connection);

        void add_channel(
                int id,
                std::shared_ptr<rtc::DataChannel> channel,
                std::optional<PeerMessageCallback> callback = std::nullopt
        );

        bool contains_peer(int id);

        std::shared_ptr<rtc::PeerConnection> get_peer(int id);

        std::vector<std::shared_ptr<KrapiRTCDataChannel>> get_channels();

        void broadcast(
                PeerMessage message,
                std::optional<PeerMessageCallback> callback = std::nullopt,
                bool include_light_nodes = false
        );

        std::shared_future<PeerMessage> send_message(
                int id,
                PeerMessage message,
                std::optional<PeerMessageCallback> callback = std::nullopt
        );
    };


} // krapi