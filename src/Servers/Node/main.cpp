//
// Created by mythi on 12/10/22.
//
#include "cxxopts.hpp"
#include "fmt/core.h"
#include "NodeManager.h"
#include "ParsingUtils.h"
#include "Message.h"
#include "Response.h"
#include "httplib.h"

krapi::Response send_discovery_message(
        httplib::Client &client,
        const std::string &url,
        const krapi::Message &message
) {

    auto res = client.Post("/", to_string(nlohmann::json(message)), "application/json");
    auto res_json = nlohmann::json::parse(res->body);
    spdlog::info("Discovery responded with {}", res_json.dump(4));
    return res_json.get<krapi::Response>();
}

int main(int argc, const char **argv) {

    cxxopts::Options options("KrapiNode", "A node for contribuitng to the krapi chain");
    options.add_options()
            ("config",
             "Server configuration file path",
             cxxopts::value<std::string>()->default_value("config.json")
            );
    auto parsed_args = options.parse(argc, argv);
    auto config_path = parsed_args["config"].as<std::string>();

    auto config = krapi::parse_node_config_file(config_path);
    auto my_uri = fmt::format("{}:{}", config.server_host, config.server_port);

    using namespace ix;
    WebSocketServer ws_server(config.server_port, config.server_host);
    ws_server.setOnClientMessageCallback(
            [](
                    const std::shared_ptr<ConnectionState> &state,
                    WebSocket &ws,
                    const WebSocketMessagePtr &message
            ) {
                if (message->type == WebSocketMessageType::Open) {
                    spdlog::info("{} Just connected", ws.getUrl());
                } else if (message->type == WebSocketMessageType::Error) {
                    spdlog::info(
                            "REASON: {}, STATUS_CODE: {}",
                            message->errorInfo.reason,
                            message->errorInfo.http_status
                    );
                } else if (message->type == WebSocketMessageType::Message) {
                    // handle network messages.
                }
            }
    );
    auto res = ws_server.listenAndStart();
    if (!res) {
        // Error handling
        return 1;
    }
    ws_server.start();
    auto discovery_url = fmt::format("http://127.0.0.1:7005");

    auto client = httplib::Client(discovery_url);

    spdlog::info("Sending node discovery request");
    auto dm_nodes_res = send_discovery_message(client, discovery_url, krapi::Message::DiscoverNodes);
    spdlog::info("Sending txpool discovery request");
    auto tx_pools_res = send_discovery_message(client, discovery_url, krapi::Message::DiscoverTxPools);


    auto node_uirs = dm_nodes_res.content.get<std::vector<std::string>>();
    spdlog::info("Received {} node uris", fmt::join(node_uirs, ","));

    auto pool_uris = tx_pools_res.content.get<std::vector<std::string>>();
    spdlog::info("Received {} txpool uris", fmt::join(pool_uris, ","));

    krapi::NodeManager manager(my_uri, node_uirs, pool_uris);
    manager.connect_to_nodes();

    manager.wait();
}