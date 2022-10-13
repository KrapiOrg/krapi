//
// Created by mythi on 13/10/22.
//

#ifndef RSPNS_RESPONSE_TXDISCOVERYRSP_H
#define RSPNS_RESPONSE_TXDISCOVERYRSP_H

#include <utility>

#include "nlohmann/json.hpp"
#include "ResponseBase.h"

namespace krapi {
    struct TxDiscoveryRsp : public ResponseBase {
        std::vector<std::string> hosts;
    public:
        explicit TxDiscoveryRsp(std::vector<std::string> hosts) :
                ResponseBase("tx_discovery_response"),
                hosts(std::move(hosts)) {}

        [[nodiscard]]
        nlohmann::json to_json() const override {

            auto json = ResponseBase::to_json();
            json["hosts"] = hosts;
            return json;
        }
    };
}
#endif //RSPNS_RESPONSE_TXDISCOVERYRSP_H
