//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_NETWORKMESSAGEHANDELER_H
#define KRAPI_MODELS_NETWORKMESSAGEHANDELER_H

#include "ixwebsocket/IXWebSocketMessage.h"
#include "spdlog/spdlog.h"

namespace krapi {

    struct NetworkMessageHandeler {

        const ix::WebSocket &socket;

        void operator()(const ix::WebSocketMessagePtr &msg) {

            using namespace ix;

            if (msg->type == WebSocketMessageType::Open) {

                spdlog::info("Connected to host {}", socket.getUrl());

            } else if (msg->type == WebSocketMessageType::Error) {
                spdlog::error("Host {} unreachable", socket.getUrl());

                if (!msg->str.empty())
                    spdlog::error("{}", msg->str);
            }
        }
    };

} // krapi

#endif //KRAPI_MODELS_NETWORKMESSAGEHANDELER_H
