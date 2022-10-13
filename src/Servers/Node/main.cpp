//
// Created by mythi on 12/10/22.
//
#include <fstream>
#include <tuple>
#include "cxxopts.hpp"
#include "ixwebsocket/IXWebSocketServer.h"
#include "fmt/core.h"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"
#include "NetworkManager.h"
#include "Server.h"


std::tuple<int, std::string, std::vector<std::string>> parse_config_file(std::string_view path) {

    int port;
    std::string host;
    std::vector<std::string> network_uris;
    std::ifstream config_file(path.data());

    if (!config_file.is_open()) {
        spdlog::error("Failed to open configuration file {}", path);
        exit(1);
    }

    auto parsed_config_file = nlohmann::json::parse(config_file);
    parsed_config_file["server_port"].get_to(port);
    parsed_config_file["server_host"].get_to(host);
    parsed_config_file["connect_to"].get_to(network_uris);

    return std::make_tuple(port, host, network_uris);
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

    auto [server_port, server_host, network_uris] = parse_config_file(config_path);

    krapi::NetworkManager network_manager{network_uris};
    network_manager.start();

    krapi::Server server{server_port, server_host};
    server.start();
    server.wait();
}