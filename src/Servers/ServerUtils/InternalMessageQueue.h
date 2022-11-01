//
// Created by mythi on 01/11/22.
//

#ifndef SHARED_MODELS_INTERNALMESSAGEQUEUE_H
#define SHARED_MODELS_INTERNALMESSAGEQUEUE_H

#include "eventpp/eventdispatcher.h"

namespace krapi{
    enum class InternalMessage {
        Start,
        Block,
        Stop
    };
    using InternalMessageQueue = eventpp::EventDispatcher<InternalMessage, void()>;
}
#endif //SHARED_MODELS_INTERNALMESSAGEQUEUE_H
