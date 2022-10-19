//
// Created by mythi on 15/10/22.
//

#ifndef RSPNS_MODELS_DISCOVERNODESMSG_H
#define RSPNS_MODELS_DISCOVERNODESMSG_H

#include "MessageBase.h"

namespace krapi {
    struct DiscoverNodesMsg : public MessageBase {
        explicit DiscoverNodesMsg() : MessageBase("discover_nodes_msg") {}
    };
}
#endif //RSPNS_MODELS_DISCOVERNODESMSG_H
