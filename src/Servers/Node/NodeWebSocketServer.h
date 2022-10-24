//
// Created by mythi on 24/10/22.
//

#ifndef SHARED_MODELS_NODEWEBSOCKETSERVER_H
#define SHARED_MODELS_NODEWEBSOCKETSERVER_H

#include "ixwebsocket/IXWebSocketServer.h"
#include "nlohmann/json.hpp"
#include "NodeMessage.h"
#include "spdlog/spdlog.h"

namespace krapi {

    class NodeWebSocketServer {
        ix::WebSocketServer server;
        int identity;

        void onMessage(
                const std::shared_ptr<ix::ConnectionState> &state,
                ix::WebSocket &ws,
                const ix::WebSocketMessagePtr &message
        ) {

            using namespace ix;
            if (message->type == WebSocketMessageType::Open) {
                spdlog::info("{}:{} Just connected", state->getRemoteIp(), state->getRemotePort());
            } else if (message->type == WebSocketMessageType::Error) {
                spdlog::info(
                        "REASON: {}, STATUS_CODE: {}",
                        message->errorInfo.reason,
                        message->errorInfo.http_status
                );
            } else if (message->type == WebSocketMessageType::Message) {
                auto msg_json = nlohmann::json::parse(message->str);
                auto msg = msg_json.get<krapi::NodeMessage>();
                auto rsp = krapi::NodeMessage{};
                if (msg.type == krapi::NodeMessageType::NodeIdentityRequest) {
                    rsp = krapi::NodeMessage{
                            krapi::NodeMessageType::NodeIdentityReply,
                            identity
                    };
                }

                ws.send(nlohmann::json(rsp).dump());
            }
        }


    public:
        NodeWebSocketServer(
                const std::string &host,
                int port,
                int identity
        ) : server(port, host),
            identity(identity) {

            server.setOnClientMessageCallback(
                    [this](auto &&a, auto &&b, auto &&c) {
                        onMessage(
                                std::forward<decltype(a)>(a),
                                std::forward<decltype(b)>(b),
                                std::forward<decltype(c)>(c)
                        );
                    }
            );
            auto res = server.listenAndStart();
            if (!res) {
                spdlog::error("Failed to start websocket server");
                exit(1);
            }
            spdlog::info("Started");
        }

        ~NodeWebSocketServer() {

            spdlog::info("Stopped");
            server.stop();
        }
    };

} // krapi

#endif //SHARED_MODELS_NODEWEBSOCKETSERVER_H
