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
        int ws_server_port{};
        int http_server_port{};
        std::string server_host{};
        std::string discovery_host{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                NodeServerConfig,
                ws_server_port,
                http_server_port,
                server_host,
                discovery_host
        )

    };

    struct DiscoveryServerConfig {
        int server_port{};
        std::string server_host{};
        std::string identity_host{};
        std::vector<std::string> node_hosts{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                DiscoveryServerConfig,
                server_port,
                server_host,
                identity_host,
                node_hosts
        )

    };

    struct IdentityServerConfig {
        int server_port{};
        std::string server_host{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                IdentityServerConfig,
                server_port,
                server_host
        )

    };

    template<typename T>
    T parse_config(std::string_view path) {

        std::ifstream config_file(path.data());

        if (!config_file.is_open()) {
            spdlog::error("Failed to open configuration file {}", path);
            exit(1);
        }

        T config;
        nlohmann::from_json(nlohmann::json::parse(config_file), config);
        return config;
    }
}
#endif //KRAPI_MODELS_PARSINGUTILS_H
