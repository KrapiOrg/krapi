//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_SERVERMESSAGEHANDELER_H
#define KRAPI_MODELS_SERVERMESSAGEHANDELER_H

#include <memory>
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXConnectionState.h"
#include "spdlog/spdlog.h"

namespace krapi {
    struct ServerMessageHandeler {
        void operator()(
                const std::shared_ptr<ix::ConnectionState> &connectionState,
                ix::WebSocket &webSocket,
                const ix::WebSocketMessagePtr &msg
        ) {
            spdlog::info("Remote ip: {}", connectionState->getRemoteIp());

            if (msg->type == ix::WebSocketMessageType::Open) {
                spdlog::info("New connection");
                spdlog::info("id: {}", connectionState->getId());
                spdlog::info("Uri: {}", msg->openInfo.uri);
                spdlog::info("Headers:");
                for (auto it: msg->openInfo.headers) {
                    spdlog::info("\t {},{}", it.first, it.second);
                }
            } else if (msg->type == ix::WebSocketMessageType::Message) {
                spdlog::info("Received: {}", msg->str);
                webSocket.send(msg->str, msg->binary);
            }
        }
    };
}

#endif //KRAPI_MODELS_SERVERMESSAGEHANDELER_H
