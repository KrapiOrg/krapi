//
// Created by mythi on 12/10/22.
//

#include "cxxopts.hpp"
#include "fmt/core.h"
#include "NodeManager.h"
#include "ParsingUtils.h"
#include "httplib.h"
#include "NodeHttpClient.h"
#include "NodeWebSocketServer.h"
#include "IdentityManager.h"


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

    auto client = krapi::NodeHttpClient(discovery_url);

    spdlog::info("Sending node discovery request");
    auto dm_nodes_res = client.post(krapi::Message::DiscoverNodes);
    spdlog::info("Sending identity discovery request");
    auto identity_res = client.post(krapi::Message::DiscoverIdentity);

    auto node_uirs = dm_nodes_res.content.get<std::vector<std::string>>();
    auto identity_uri = identity_res.content.get<std::string>();

    krapi::IdentityManager identity_manager(identity_uri);

    krapi::NodeWebSocketServer ws_server(
            config.server_host,
            config.server_port,
            identity_manager.identity()
    );

    krapi::NodeManager node_manager(
            my_uri,
            node_uirs,
            identity_manager.identity()
    );

    node_manager.connect_to_nodes();
    node_manager.wait();
}