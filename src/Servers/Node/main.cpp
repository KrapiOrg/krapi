//
// Created by mythi on 12/10/22.
//
#include "cxxopts.hpp"
#include "fmt/core.h"
#include "NetworkManager.h"
#include "Server.h"
#include "ParsingUtils.h"

int main(int argc, const char **argv) {

    cxxopts::Options options("KrapiNode", "A node for contribuitng to the krapi chain");
    options.add_options()
            ("config",
             "Server configuration file path",
             cxxopts::value<std::string>()->default_value("config.json")
            );
    auto parsed_args = options.parse(argc, argv);
    auto config_path = parsed_args["config"].as<std::string>();

    auto [server_port, server_host, hosts] = krapi::parse_config_file(config_path);

    krapi::NetworkManager network_manager{hosts};
    network_manager.start();

    krapi::Server server{server_port, server_host};
    server.start();
    server.wait();
}