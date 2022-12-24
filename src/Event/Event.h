#pragma once

#include <variant>
#include "concurrencpp/concurrencpp.h"
#include "SignalingMessage.h"
#include "PeerMessage.h"
#include "Box.h"

namespace krapi {

    using EventType = std::variant<SignalingMessageType, PeerMessageType>;
    using _Event = std::variant<Box<SignalingMessage>, Box<PeerMessage>>;

    struct Event : _Event {
        using _Event::variant;

        template<typename T>
        Box<T> get() const {

            return std::get<Box<T>>(*this);
        }
    };

    using EventPromise = concurrencpp::result_promise<Event>;
}