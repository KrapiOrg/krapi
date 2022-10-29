//
// Created by mythi on 25/10/22.
//

#ifndef NODE_NODEMESSAGEQUEUE_H
#define NODE_NODEMESSAGEQUEUE_H

#include "eventpp/eventdispatcher.h"
#include "NodeMessage.h"

namespace krapi {
    using MessageQueue = eventpp::EventDispatcher<NodeMessageType, void(const NodeMessage &)>;
    using MessageQueuePtr = std::shared_ptr<MessageQueue>;

    inline MessageQueuePtr create_message_queue() {
        return std::make_shared<MessageQueue>();
    }
}

#endif //NODE_NODEMESSAGEQUEUE_H
