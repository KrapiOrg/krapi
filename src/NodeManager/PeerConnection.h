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

namespace krapi {

    class PeerConnection {
        rtc::Configuration m_configuration;
        std::shared_ptr<rtc::PeerConnection> m_peer_connection;
        std::shared_ptr<rtc::DataChannel> m_datachannel;
        std::shared_ptr<SignalingClient> m_signaling_client;
        std::unordered_map<std::string, concurrencpp::result_promise<PeerMessage>> m_promises;
        std::string m_identity;
        bool m_initialized;
        eventpp::EventDispatcher<PeerMessageType, void(PeerMessage)> m_dispatcher;

        static concurrencpp::result<std::shared_ptr<rtc::DataChannel>> wait_for_open(std::shared_ptr<rtc::DataChannel>);

        void setup_internal_listiners();
        void setup_peer_connection();

    public:

        explicit PeerConnection(
                std::string
                identity,
                std::shared_ptr<SignalingClient> signaling_client
        );

        explicit PeerConnection(
                std::string identity,
                std::shared_ptr<SignalingClient> signaling_client,
                std::string description
        );

        concurrencpp::result<void> initialize_channel(std::shared_ptr<rtc::DataChannel>);

        concurrencpp::result<void> create_datachannel();

        concurrencpp::result<PeerMessage> send(PeerMessage);

        concurrencpp::result<PeerState> request_state();

        concurrencpp::result<PeerType> request_type();

        void set_remote_description(std::string);

        void add_remote_candidate(std::string);
    };
} // krapi
