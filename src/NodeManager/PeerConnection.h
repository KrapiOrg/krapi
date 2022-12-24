//
// Created by mythi on 19/12/22.
//

#pragma once

#include <memory>
#include <sstream>

#include "rtc/peerconnection.hpp"

#include "SignalingMessage.h"
#include "SignalingClient.h"
#include "PeerType.h"
#include "PeerState.h"
#include "PeerMessage.h"
#include "NotNull.h"

namespace krapi {

    using RTCPeerConnection = std::shared_ptr<rtc::PeerConnection>;
    using RTCDataChannel = std::shared_ptr<rtc::DataChannel>;
    using RTCDataChannelResult = concurrencpp::result<std::shared_ptr<rtc::DataChannel>>;

    class PeerConnection {

        NotNull<SignalingClient *> m_signaling_client;
        NotNull<EventQueue *> m_event_queue;
        RTCPeerConnection m_peer_connection;
        RTCDataChannel m_datachannel;

        std::string m_identity;
        bool m_initialized;

        static RTCDataChannelResult wait_for_open(RTCDataChannel);

        void on_local_candidate(rtc::Candidate);

        void on_local_description(rtc::Description);

        void on_data_channel(std::shared_ptr<rtc::DataChannel>);

    public:

        explicit PeerConnection(
                NotNull<EventQueue *> event_queue,
                NotNull<SignalingClient *> signaling_client,
                std::string identity
        );

        explicit PeerConnection(
                NotNull<EventQueue *> event_queue,
                NotNull<SignalingClient *> signaling_client,
                std::string identity,
                std::string description
        );

        template<typename ...UU>
        static inline std::shared_ptr<PeerConnection> create(UU &&... params) {
            return std::make_shared<PeerConnection>(std::forward<UU>(params)...);
        }

        concurrencpp::result<void> initialize_channel(RTCDataChannel);

        concurrencpp::result<void> create_datachannel();

        concurrencpp::result<Event> send(Box<PeerMessage>);

        concurrencpp::result<PeerState> request_state();

        concurrencpp::result<PeerType> request_type();

        void set_remote_description(std::string);

        void add_remote_candidate(std::string);
    };
} // krapi
