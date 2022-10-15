//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_DISCOVERTXPOOLS_H
#define KRAPI_MODELS_DISCOVERTXPOOLS_H
#include "MessageBase.h"

namespace krapi {
    struct DiscoverTxPoolsMsg : public MessageBase {
        explicit DiscoverTxPoolsMsg() : MessageBase("discover_tx_pools_msg") {}
    };
}
#endif //KRAPI_MODELS_DISCOVERTXPOOLS_H
