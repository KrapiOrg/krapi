//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_NETWORKNODE_H
#define KRAPI_MODELS_NETWORKNODE_H

#include <string>
#include "ixwebsocket/IXWebSocket.h"

namespace krapi {

    class NetworkNode {
        std::string m_uri;
        ix::WebSocket socket;
    public:

        explicit NetworkNode(std::string uri);

        using Ptr = std::unique_ptr<NetworkNode>;

        template<typename ...Args>
        static Ptr create(Args &&...args) {

            return std::make_unique<NetworkNode>(std::forward<Args>(args)...);
        }

        void start();
    };

} // krapi

#endif //KRAPI_MODELS_NETWORKNODE_H
