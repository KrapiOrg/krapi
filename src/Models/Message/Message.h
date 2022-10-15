//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_MESSAGE_H
#define KRAPI_MODELS_MESSAGE_H

#include <variant>
#include "AckMsg.h"
#include "CreateTxMsg.h"
#include "DiscoverTxPoolsMsg.h"

namespace krapi {
    using Message = std::variant<AckMsg, CreateTxMsg, DiscoverTxPoolsMsg>;
}
#endif //KRAPI_MODELS_MESSAGE_H
