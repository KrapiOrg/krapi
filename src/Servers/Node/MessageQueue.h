//
// Created by mythi on 25/10/22.
//

#ifndef NODE_NODEMESSAGEQUEUE_H
#define NODE_NODEMESSAGEQUEUE_H

#include "eventpp/eventdispatcher.h"
#include "NodeMessage.h"

namespace krapi {
    using NodeMessageQueue = eventpp::EventDispatcher<NodeMessageType, void(const NodeMessage &)>;
    using NodeMessageQueuePtr = std::shared_ptr<NodeMessageQueue>;

    inline NodeMessageQueuePtr create_message_queue() {
        return std::make_shared<NodeMessageQueue>();
    }
}

#endif //NODE_NODEMESSAGEQUEUE_H
