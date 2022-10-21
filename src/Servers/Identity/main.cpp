//
// Created by mythi on 21/10/22.
//

#include "cxxopts.hpp"
#include "ixwebsocket/IXWebSocketServer.h"
#include <mutex>

#include "ParsingUtils.h"
#include "Response.h"

int main(int argc, const char **argv) {

    cxxopts::Options options("KrapiIdentityServer",
                             "A server that assigns unique identifiers to nodes");
    options.add_options()
            ("config",
             "Server configuration file path",
             cxxopts::value<std::string>()->default_value("identity_config.json")
            );
    auto parsed_args = options.parse(argc, argv);
    auto config_path = parsed_args["config"].as<std::string>();

    auto config = krapi::parse_config<krapi::IdentityServerConfig>(config_path);

    spdlog::info("Starting Identity Server on {}:{}", config.server_host, config.server_port);

    using namespace ix;
    WebSocketServer server(config.server_port, config.server_host);

    std::mutex identity_map_mutex;
    std::map<int, std::shared_ptr<WebSocket>> identity_map;
    for (int i = 1; i <= 100; i++) {
        identity_map[i] = nullptr;
    }

    server.setOnClientMessageCallback(
            [&](
                    const std::shared_ptr<ConnectionState> &state,
                    WebSocket &ws,
                    const WebSocketMessagePtr &msg
            ) {
                if (msg->type == WebSocketMessageType::Open) {

                    spdlog::info("New connection", ws.getUrl());

                    std::lock_guard<std::mutex> lk(identity_map_mutex);
                    int identity = -1;
                    for (const auto &[k, v]: identity_map) {
                        if (v == nullptr) {
                            identity = k;
                            break;
                        }
                    }
                    krapi::Response res;
                    if (identity == -1) {
                        spdlog::warn("Failed to assign an identity");

                        res = krapi::Response{krapi::ResponseType::IdentityError};
                    } else {

                        for (const auto &client: server.getClients()) {
                            if (client.get() == &ws) {
                                identity_map[identity] = client;
                                break;
                            }
                        }
                        spdlog::info("Assigned {}", identity);
                        res = krapi::Response{krapi::ResponseType::IdentityFound, identity};
                    }
                    auto response_json = nlohmann::json(res);
                    ws.send(response_json.dump());
                } else if (msg->type == WebSocketMessageType::Close || msg->type == WebSocketMessageType::Error) {
                    std::lock_guard<std::mutex> lk(identity_map_mutex);
                    for (auto &[k, v]: identity_map) {
                        if (v.get() == &ws) {
                            spdlog::info("Removed Identity {}", k);
                            v = nullptr;
                            break;
                        }
                    }
                }
            }
    );

    auto res = server.listenAndStart();
    if (!res) {
        spdlog::error("Failed to setup identity server");
        return 1;
    }

    server.wait();
}