//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_PARSINGUTILS_H
#define KRAPI_MODELS_PARSINGUTILS_H

#include <fstream>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"


namespace krapi {
    struct NodeServerConfig {
        int server_port{};
        std::string server_host{};
        std::string discovery_host{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                NodeServerConfig,
                server_port,
                server_host,
                discovery_host
        )

    };


    NodeServerConfig parse_node_config_file(std::string_view path) {

        std::ifstream config_file(path.data());

        if (!config_file.is_open()) {
            spdlog::error("Failed to open configuration file {}", path);
            exit(1);
        }

        NodeServerConfig config;
        nlohmann::from_json(nlohmann::json::parse(config_file), config);


        return config;
    }

    struct DiscoveryServerConfig {
        int server_port{};
        std::string server_host{};
        std::vector<std::string> node_hosts{};
        std::vector<std::string> pool_hosts{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                DiscoveryServerConfig,
                server_port,
                server_host,
                node_hosts,
                pool_hosts
        )

    };

    DiscoveryServerConfig parse_discovery_config_file(std::string_view path) {


        std::ifstream config_file(path.data());

        if (!config_file.is_open()) {
            spdlog::error("Failed to open configuration file {}", path);
            exit(1);
        }

        DiscoveryServerConfig config;
        nlohmann::from_json(nlohmann::json::parse(config_file), config);

        return config;
    }
}
#endif //KRAPI_MODELS_PARSINGUTILS_H
