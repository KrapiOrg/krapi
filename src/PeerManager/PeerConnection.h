#pragma once

#include <memory>
#include <sstream>
#include <string>

#include "EventQueue.h"
#include "eventpp/utilities/scopedremover.h"
#include "rtc/peerconnection.hpp"

#include "PeerMessage.h"
#include "PeerState.h"
#include "PeerType.h"
#include "SignalingClient.h"
#include "SignalingMessage.h"

namespace krapi {

  using RTCPeerConnection = std::shared_ptr<rtc::PeerConnection>;
  using RTCDataChannel = std::shared_ptr<rtc::DataChannel>;

  class PeerConnection {

    SignalingClientPtr m_signaling_client;
    EventQueuePtr m_event_queue;
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
      EventQueuePtr event_queue,
      SignalingClientPtr signaling_client,
      std::string identity
    );

    explicit PeerConnection(
      EventQueuePtr event_queue,
      SignalingClientPtr signaling_client,
      std::string identity,
      std::string sdp,
      std::string type
    );

    template<typename... UU>
    static inline std::shared_ptr<PeerConnection> create(UU &&...params) {
      return std::make_shared<PeerConnection>(std::forward<UU>(params)...);
    }

    void initialize_channel(RTCDataChannel);

    [[nodiscard]] bool send_and_forget(Box<PeerMessage>) const;

    void set_remote_description(std::string, std::string);

    concurrencpp::result<PeerState> state();

    concurrencpp::result<PeerType> type();

    concurrencpp::result<Event> wait_for_datachannel_open(std::string);

    ~PeerConnection();
  };
  using PeerConnectionPtr = std::shared_ptr<PeerConnection>;
}// namespace krapi
