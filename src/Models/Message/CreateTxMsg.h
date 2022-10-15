//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_CREATETXMESSAGE_H
#define KRAPI_MODELS_CREATETXMESSAGE_H

#include "MessageBase.h"

namespace krapi {
    struct CreateTxMsg : public MessageBase {
        explicit CreateTxMsg() : MessageBase("create_tx") {}

        [[nodiscard]]
        nlohmann::json to_json() const override {

            return MessageBase::to_json();
        }
    };
}

#endif //KRAPI_MODELS_CREATETXMESSAGE_H
