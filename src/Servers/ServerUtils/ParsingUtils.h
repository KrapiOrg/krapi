//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_PARSINGUTILS_H
#define KRAPI_MODELS_PARSINGUTILS_H

#include <fstream>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"


namespace krapi {
    using ServerHost = std::pair<std::string, int>;
    using ServerHosts = std::vector<ServerHost>;

    struct NodeServerConfig {
        ServerHost ws_server_host{};
        ServerHost http_server_host{};
        ServerHost discovery_host{};
        std::filesystem::path blockchain_path;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                NodeServerConfig,
                ws_server_host,
                http_server_host,
                discovery_host,
                blockchain_path
        )

    };

    struct DiscoveryServerConfig {
        ServerHost discovery_host{};
        ServerHost identity_host{};
        std::vector<ServerHost> network_hosts{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                DiscoveryServerConfig,
                discovery_host,
                identity_host,
                network_hosts
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
