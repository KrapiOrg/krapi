//
// Created by mythi on 13/10/22.
//
#include "ixwebsocket/IXWebSocket.h"
#include "spdlog/spdlog.h"
#include "Message/DiscoverTxPools.h"
#include "Response/Response.h"
#include "Overload.h"
#include "Response/JsonToResponseConverter.h"

int main() {

    ix::WebSocket discovery_socket;
    discovery_socket.setUrl("ws://127.0.0.1:8080");

    discovery_socket.setOnMessageCallback(
            [&discovery_socket](const ix::WebSocketMessagePtr &message) {
                if (message->type == ix::WebSocketMessageType::Open) {
                    spdlog::info("Connected to discovery server");
                    spdlog::info("Sending discovery request");
                    discovery_socket.sendText(krapi::DiscoverTxPools{}.to_string());
                } else if (message->type == ix::WebSocketMessageType::Error) {

                    spdlog::error("{}", message->errorInfo.reason);
                } else if (message->type == ix::WebSocketMessageType::Message) {
                    auto msg = std::invoke(krapi::JsonToResponseConverter{}, message->str);
                    std::visit(
                            Overload{
                                    [](auto) {},
                                    [](const krapi::TxDiscoveryRsp &rsp)  {
                                        spdlog::info("Discovered {}", rsp.to_string());
                                    }
                            },
                            msg
                    );
                }
            }
    );

    discovery_socket.run();


//    ix::WebSocket pool_socket;
//    pool_socket.setUrl("ws://127.0.0.1:7070");
//
//    pool_socket.setOnMessageCallback(
//            [&pool_socket](const ix::WebSocketMessagePtr &message) {
//                if (message->type == ix::WebSocketMessageType::Error) {
//                    spdlog::error("Pool {} is unreachable, reason: {}", pool_socket.getUrl(), message->str);
//                } else if (message->type == ix::WebSocketMessageType::Open) {
//                    spdlog::info("Connected to node {} is", pool_socket.getUrl());
//                }
//            }
//    );
//    pool_socket.run();
}