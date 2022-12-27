#pragma once

#include <variant>
#include "SignalingMessage.h"
#include "PeerMessage.h"
#include "InternalMessage.h"
#include "InternalNotification.h"
#include "Box.h"

namespace krapi {

    using EventType = std::variant<
            SignalingMessageType,
            PeerMessageType,
            InternalMessageType,
            InternalNotificationType
    >;
    using EventVariant = std::variant<
            Box<SignalingMessage>,
            Box<PeerMessage>,
            Box<InternalMessage<SignalingMessage>>,
            Box<InternalMessage<PeerMessage>>,
            Box<InternalNotification<std::string>>,
            Box<InternalNotification<Empty>>
    >;

    struct Event : EventVariant {
        using EventVariant::variant;

        template<typename T>
        Box<T> get() const {

            return std::get<Box<T>>(*this);
        }
    };
}