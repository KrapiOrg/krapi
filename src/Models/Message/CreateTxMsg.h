//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_CREATETXMESSAGE_H
#define KRAPI_MODELS_CREATETXMESSAGE_H

#include "MessageBase.h"

namespace krapi {
    struct CreateTxMsg : public MessageBase {
        explicit CreateTxMsg() : MessageBase("create_tx_msg") {}
    };
}

#endif //KRAPI_MODELS_CREATETXMESSAGE_H
