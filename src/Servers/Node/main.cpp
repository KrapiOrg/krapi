//
// Created by mythi on 12/10/22.
//

#include "cxxopts.hpp"
#include "fmt/core.h"
#include "NodeManager.h"
#include "ParsingUtils.h"
#include "NodeHttpClient.h"

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

    auto client = krapi::NodeHttpClient(fmt::format("http://{}", config.discovery_host));

    spdlog::info("Sending node discovery request");
    auto dm_nodes_res = client.post(krapi::Message::DiscoverNodes);
    spdlog::info("Sending identity discovery request");
    auto identity_res = client.post(krapi::Message::DiscoverIdentity);

    auto node_uirs = dm_nodes_res.content.get<std::vector<std::string>>();
    auto identity_uri = identity_res.content.get<std::string>();

    krapi::NodeManager node_manager(
            config.server_host,
            config.ws_server_port,
            config.http_server_port,
            node_uirs,
            identity_uri
    );

    node_manager.connect_to_nodes();
    node_manager.wait();
}