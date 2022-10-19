//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_RESPONSE_H
#define KRAPI_MODELS_RESPONSE_H

#include <variant>
#include "TxPoolDiscoveryRsp.h"
#include "ErrorRsp.h"
#include "NodeDiscoveryRsp.h"

namespace krapi {
    using Response = std::variant<TxPoolDiscoveryRsp, ErrorRsp, NodesDiscoveryRsp>;
}
#endif //KRAPI_MODELS_RESPONSE_H
