//
// Created by mythi on 19/10/22.
//

#ifndef RSPNS_MODELS_NODEMESSAGE_H
#define RSPNS_MODELS_NODEMESSAGE_H

#include "nlohmann/json.hpp"

namespace krapi {

    enum class NodeMessageType {
        Stop,
        Broadcast
    };

    struct NodeMessage {
        NodeMessageType type{};
        std::string str{};
        std::vector<std::string> black_list{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NodeMessage, type, str, black_list)
    };

} // krapi

#endif //RSPNS_MODELS_NODEMESSAGE_H
