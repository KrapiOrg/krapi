//
// Created by mythi on 24/10/22.
//

#ifndef NODE_NODEIDENTITYMANAGER_H
#define NODE_NODEIDENTITYMANAGER_H

#include <future>
#include "ixwebsocket/IXWebSocket.h"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "fmt/format.h"

#include "Response.h"
#include "ParsingUtils.h"

namespace krapi {

    class IdentityManager {
        int m_identity;

        std::promise<int> identity_promise;

        ServerHost m_host;
        ix::WebSocket ws;

        void onMessage(const ix::WebSocketMessagePtr &msg);

    public:
        explicit IdentityManager(ServerHost host);

        [[nodiscard]]
        int identity() const;

        ~IdentityManager();
    };

} // krapi

#endif //NODE_NODEIDENTITYMANAGER_H
