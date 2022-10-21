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
    spdlog::info("Discovery responded with {}", res_json.dump());
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

    auto config = krapi::parse_config<krapi::NodeServerConfig>(config_path);
    auto my_uri = fmt::format("{}:{}", config.server_host, config.server_port);


    auto discovery_url = fmt::format("http://127.0.0.1:7005");

    auto client = httplib::Client(discovery_url);

    spdlog::info("Sending node discovery request");
    auto dm_nodes_res = send_discovery_message(client, discovery_url, krapi::Message::DiscoverNodes);
    spdlog::info("Sending txpool discovery request");
    auto tx_pools_res = send_discovery_message(client, discovery_url, krapi::Message::DiscoverTxPools);
    spdlog::info("Sending identity discovery request");
    auto identity_res = send_discovery_message(client, discovery_url, krapi::Message::DiscoverIdentity);

    auto node_uirs = dm_nodes_res.content.get<std::vector<std::string>>();
    auto pool_uris = tx_pools_res.content.get<std::vector<std::string>>();
    auto identity_uri = identity_res.content.get<std::string>();

    krapi::NodeManager manager(my_uri, identity_uri, node_uirs, pool_uris);

    using namespace ix;

    WebSocketServer ws_server(config.server_port, config.server_host);
    ws_server.setOnClientMessageCallback(
            [&manager](
                    const std::shared_ptr<ConnectionState> &state,
                    WebSocket &ws,
                    const WebSocketMessagePtr &message
            ) {
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
                                manager.identity()
                        };
                    }

                    ws.send(nlohmann::json(rsp).dump());
                }
            }
    );
    auto res = ws_server.listenAndStart();
    if (!res) {
        // Error handling
        return 1;
    }
    manager.connect_to_nodes();
    manager.wait();
    ws_server.wait();
}