//
// Created by mythi on 12/10/22.
//

#include "cxxopts.hpp"
#include "fmt/core.h"
#include "NodeManager.h"
#include "ParsingUtils.h"
#include "HttpClient.h"

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

    auto client = krapi::HttpClient(config.discovery_host);

    spdlog::info("Sending node discovery request");
    auto dm_nodes_res = client.post("/", krapi::Message::DiscoverNodes);
    spdlog::info("Sending identity discovery request");
    auto identity_res = client.post("/", krapi::Message::DiscoverIdentity);

    auto identity_host = identity_res.content.get<krapi::ServerHost>();
    auto network_hosts = dm_nodes_res.content.get<krapi::ServerHosts>();

    krapi::NodeManager node_manager(
            config.ws_server_host,
            config.http_server_host,
            identity_host,
            network_hosts
    );

    node_manager.start_http_server();
    node_manager.start_ws_server();
    node_manager.connect_to_nodes();
    node_manager.wait();
}