//
// Created by mythi on 13/10/22.
//


#include "cxxopts.hpp"
#include "nlohmann/json.hpp"
#include "ParsingUtils.h"
#include "Message/JsonToMessageConverter.h"
#include "Overload.h"
#include "Response/Response.h"

int main(int argc, const char **argv) {

    cxxopts::Options options("KrapiDiscoveryNode",
                             "A server that keeps an updated list of active nodes and transaction pools");
    options.add_options()
            ("config",
             "Server configuration file path",
             cxxopts::value<std::string>()->default_value("discovery_config.json")
            );
    auto parsed_args = options.parse(argc, argv);
    auto config_path = parsed_args["config"].as<std::string>();

    auto [server_port, server_host, hosts] = krapi::parse_config_file(config_path);
    auto h = hosts;
    ix::WebSocketServer server(server_port, server_host);

    server.setOnClientMessageCallback(
            [&hosts = h](
                    const std::shared_ptr<ix::ConnectionState> &connectionState,
                    ix::WebSocket &webSocket,
                    const ix::WebSocketMessagePtr &msg
            ) {
                if (msg->type == ix::WebSocketMessageType::Open) {
                    spdlog::info("Client {} connected", webSocket.getUrl());
                } else if (msg->type == ix::WebSocketMessageType::Error) {
                    spdlog::error("{}", msg->str);

                } else if (msg->type == ix::WebSocketMessageType::Message) {
                    spdlog::warn("Recivied msg {}", msg->str);
                    auto json = nlohmann::json::parse(msg->str);
                    auto message = std::invoke(krapi::JsonToMessageConverter{}, json);
                    auto response = std::visit<nlohmann::json>(
                            Overload{
                                    [](auto) {
                                        return krapi::ErrorRsp{}.to_json();
                                    },
                                    [&hosts](const krapi::DiscoverTxPools &) {
                                        // TODO: instead of simply sending out a static list the Discover server should keep track of avaliable pools
                                        return krapi::TxDiscoveryRsp{hosts}.to_json();
                                    }
                            }, message
                    );
                    spdlog::info("Sending {} to {}", response.dump(), webSocket.getUrl());
                    webSocket.send(response.dump());
                }
            }
    );
    auto res = server.listenAndStart();
    if (!res) {
        spdlog::error("");
        return 1;
    }
    spdlog::info("Started Discovery Server on {}:{}", server.getHost(), server.getPort());
    server.wait();
}