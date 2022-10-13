//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_RESPONSE_H
#define KRAPI_MODELS_RESPONSE_H

#include <variant>
#include "TxDiscoveryRsp.h"
#include "ErrorRsp.h"

namespace krapi {
    using Response = std::variant<TxDiscoveryRsp, ErrorRsp>;
}
#endif //KRAPI_MODELS_RESPONSE_H
