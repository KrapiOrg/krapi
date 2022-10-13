//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_ACKMESSAGE_H
#define KRAPI_MODELS_ACKMESSAGE_H

#include "MessageBase.h"

namespace krapi {
    struct AckMessage : public MessageBase {
        explicit AckMessage() : MessageBase("ack") {}

        [[nodiscard]]
        nlohmann::json to_json() const override {

            return MessageBase::to_json();
        }
    };
}
#endif //KRAPI_MODELS_ACKMESSAGE_H
