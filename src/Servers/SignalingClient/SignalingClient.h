//
// Created by mythi on 02/12/22.
//

#pragma once

#include "EventQueue.h"
#include "eventpp/utilities/scopedremover.h"
#include "rtc/websocket.hpp"
#include "spdlog/spdlog.h"

namespace krapi {

  class SignalingClient {

   public:
    explicit SignalingClient(EventQueuePtr);

    [[nodiscard]] std::string identity() const noexcept;

    [[nodiscard]] concurrencpp::result<void> initialize();

    template<typename... UU>
    [[nodiscard]] static std::shared_ptr<SignalingClient> create(UU &&...uu) {

      return std::make_shared<SignalingClient>(std::forward<UU>(uu)...);
    }

   private:
    EventQueuePtr m_event_queue;
    std::unique_ptr<rtc::WebSocket> m_ws;
    std::string m_identity;
    eventpp::ScopedRemover<EventQueueType> m_subscription_remover;
  };

  using SignalingClientPtr = std::shared_ptr<SignalingClient>;
}// namespace krapi
