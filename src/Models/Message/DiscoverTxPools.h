//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_DISCOVERTXPOOLS_H
#define KRAPI_MODELS_DISCOVERTXPOOLS_H
#include "MessageBase.h"

namespace krapi {
    struct DiscoverTxPools : public MessageBase {
        explicit DiscoverTxPools() : MessageBase("discover_tx_pools") {}

        [[nodiscard]]
        nlohmann::json to_json() const override {

            return MessageBase::to_json();
        }
    };
}
#endif //KRAPI_MODELS_DISCOVERTXPOOLS_H
