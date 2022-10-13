//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_PARSINGUTILS_H
#define KRAPI_MODELS_PARSINGUTILS_H

#include <fstream>
#include "ixwebsocket/IXWebSocketServer.h"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"


namespace krapi {
    std::tuple<int, std::string, std::vector<std::string>> parse_config_file(std::string_view path) {

        int port;
        std::string host;
        std::vector<std::string> hosts;
        std::ifstream config_file(path.data());

        if (!config_file.is_open()) {
            spdlog::error("Failed to open configuration file {}", path);
            exit(1);
        }

        auto parsed_config_file = nlohmann::json::parse(config_file);
        parsed_config_file["server_port"].get_to(port);
        parsed_config_file["server_host"].get_to(host);
        parsed_config_file["hosts"].get_to(hosts);

        return std::make_tuple(port, host, hosts);
    }

}
#endif //KRAPI_MODELS_PARSINGUTILS_H
