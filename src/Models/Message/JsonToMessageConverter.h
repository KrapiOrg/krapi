//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_MESSAGETOJSONCONVERTER_H
#define KRAPI_MODELS_MESSAGETOJSONCONVERTER_H

#include "Message.h"
#include "nlohmann/json.hpp"

namespace krapi {
    struct JsonToMessageConverter {

        Message operator()(std::string_view str) {
            auto json = nlohmann::json::parse(str);
            const auto &type = json["type"];
            if (type == "ack") {
                return AckMessage{};
            } else if (type == "discover_tx_pools") {
                return DiscoverTxPools{};
            }
            return CreateTxMessage{};
        }

    };
}
#endif //KRAPI_MODELS_MESSAGETOJSONCONVERTER_H
