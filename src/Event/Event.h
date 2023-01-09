#pragma once

#include "InternalMessage.h"
#include "InternalNotification.h"
#include <variant>

namespace krapi {

  using EventType = std::variant<
    SignalingMessageType,
    PeerMessageType,
    InternalMessageType,
    InternalNotificationType>;
  using EventVariant = std::variant<
    Box<SignalingMessage>,
    Box<PeerMessage>,
    Box<InternalMessage<SignalingMessage>>,
    Box<InternalMessage<PeerMessage>>,
    Box<InternalNotification<std::string>>,
    Box<InternalNotification<int>>,
    Box<InternalNotification<Transaction>>,
    Box<InternalNotification<Block>>,
    Box<InternalNotification<Empty>>>;

  struct Event : EventVariant {
    using EventVariant::variant;

    bool listenable;

    explicit Event(bool listenable = false) : listenable(listenable) {}

    template<typename T>
    Box<T> get() const {

      return std::get<Box<T>>(*this);
    }
  };
}// namespace krapi