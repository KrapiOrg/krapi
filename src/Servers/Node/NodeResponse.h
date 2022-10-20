//
// Created by mythi on 20/10/22.
//

#ifndef NODE_NODERESPONSE_H
#define NODE_NODERESPONSE_H

#include "nlohmann/json.hpp"

namespace krapi {

    enum class NodeResponseType {
        Def
    };

    struct NodeResponse {
        NodeResponseType type;
        nlohmann::json content;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(NodeResponse, type, content)
    };

} // krapi

#endif //NODE_NODERESPONSE_H
