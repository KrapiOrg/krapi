//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_RESPONSETOJSONCONVERTER_H
#define KRAPI_MODELS_RESPONSETOJSONCONVERTER_H

#include "Response.h"
#include "nlohmann/json.hpp"

namespace krapi {
    struct JsonToResponseConverter {

        Response operator()(std::string_view str) {
            auto json = nlohmann::json::parse(str);
            const auto &type = json["type"];
            if (type == "tx_discovery_rsp") {
                return TxDiscoveryRsp{json["hosts"]};
            }
            return ErrorRsp{};
        }

    };
}
#endif //KRAPI_MODELS_RESPONSETOJSONCONVERTER_H
