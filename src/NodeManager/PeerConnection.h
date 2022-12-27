//
// Created by mythi on 19/12/22.
//

#pragma once

#include <memory>
#include <sstream>

#include "rtc/peerconnection.hpp"
#include "eventpp/utilities/scopedremover.h"
#include "EventQueue.h"

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
        PeerType m_peer_type;
        std::atomic<PeerState> m_peer_state;
        std::string m_identity;
        eventpp::ScopedRemover<EventQueueType> m_subscription_remover;

        void on_local_candidate(rtc::Candidate);

        void on_local_description(rtc::Description);

        void on_peer_state_update(Event);

        void on_rtc_candidate(Event);

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

        void initialize_channel(RTCDataChannel);

        [[nodiscard]]
        bool send_and_forget(Box<PeerMessage>) const;

        void set_remote_description(std::string);

        concurrencpp::result<PeerState> state();

        concurrencpp::result<PeerType> type();

        concurrencpp::result<Event> wait_for_datachannel_open(std::string);

        ~PeerConnection();
    };
} // krapi
