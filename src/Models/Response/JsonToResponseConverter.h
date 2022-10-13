//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_RESPONSETOJSONCONVERTER_H
#define KRAPI_MODELS_RESPONSETOJSONCONVERTER_H

#include "Response.h"
#include "nlohmann/json.hpp"

namespace krapi {
    struct JsonToResponseConverter {

        Response operator()(const nlohmann::json &json) {

            const auto &type = json["type"];
            if (type == "tx_discovery_response") {
                return TxDiscoveryRsp{json["hosts"]};
            }
            return ErrorRsp{};
        }

    };
}
#endif //KRAPI_MODELS_RESPONSETOJSONCONVERTER_H
