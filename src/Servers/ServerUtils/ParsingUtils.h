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
        ServerHost host{};
        std::filesystem::path blockchain_path;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(
                NodeServerConfig,
                host,
                blockchain_path
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
