//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_MESSAGE_H
#define KRAPI_MODELS_MESSAGE_H

#include <variant>
#include "AckMessage.h"
#include "CreateTxMessage.h"
#include "DiscoverTxPools.h"

namespace krapi {
    using Message = std::variant<AckMessage, CreateTxMessage, DiscoverTxPools>;
}
#endif //KRAPI_MODELS_MESSAGE_H
