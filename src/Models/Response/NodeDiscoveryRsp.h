//
// Created by mythi on 15/10/22.
//

#ifndef RSPNS_MODELS_NODEDISCOVERYRSP_H
#define RSPNS_MODELS_NODEDISCOVERYRSP_H

#include "ResponseBase.h"

namespace krapi {
    struct NodesDiscoveryRsp : public ResponseBase {
    private:
        std::vector<std::string> hosts;
    public:
        explicit NodesDiscoveryRsp(std::vector<std::string> hosts) :
        ResponseBase("nodes_discovery_rsp"),
        hosts(std::move(hosts)) {}

        [[nodiscard]]
        const std::vector<std::string> &get_hosts() const {

            return hosts;
        }

        [[nodiscard]]
        nlohmann::json to_json() const override {

            auto json = ResponseBase::to_json();
            json["hosts"] = hosts;
            return json;
        }
    };
}

#endif //RSPNS_MODELS_NODEDISCOVERYRSP_H
