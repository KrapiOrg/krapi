//
// Created by mythi on 13/10/22.
//

#ifndef RSPNS_RESPONSE_TXDISCOVERYRSP_H
#define RSPNS_RESPONSE_TXDISCOVERYRSP_H

#include <utility>

#include "nlohmann/json.hpp"
#include "ResponseBase.h"

namespace krapi {
    struct TxPoolDiscoveryRsp : public ResponseBase {
    private:
        std::vector<std::string> hosts;
    public:
        explicit TxPoolDiscoveryRsp(std::vector<std::string> hosts) :
                ResponseBase("tx_discovery_rsp"),
                hosts(std::move(hosts)) {}

        [[nodiscard]]
        nlohmann::json to_json() const override {

            auto json = ResponseBase::to_json();
            json["hosts"] = hosts;
            return json;
        }

        [[nodiscard]]
        const std::vector<std::string> &get_hosts() const {

            return hosts;
        }
    };
}
#endif //RSPNS_RESPONSE_TXDISCOVERYRSP_H
