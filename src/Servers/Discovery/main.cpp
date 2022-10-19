//
// Created by mythi on 13/10/22.
//


#include "cxxopts.hpp"
#include "ParsingUtils.h"
#include "Message/JsonToMessageConverter.h"
#include "Overload.h"
#include "Response/Response.h"
#include "httplib.h"

int main(int argc, const char **argv) {

    cxxopts::Options options("KrapiDiscoveryNode",
                             "A server that keeps an updated list of active nodes and transaction pools");
    options.add_options()
            ("config",
             "Server configuration file path",
             cxxopts::value<std::string>()->default_value("discovery_config.json")
            );
    auto parsed_args = options.parse(argc, argv);
    auto config_path = parsed_args["config"].as<std::string>();

    auto config = krapi::parse_discovery_config_file(config_path);

    spdlog::info("Starting Discovery Server on {}:{}", config.server_host, config.server_port);

    httplib::Server server;
    server.Post(
            "/",
            [&](
                    const httplib::Request &req,
                    httplib::Response &res
            ) {
                auto message = std::invoke(krapi::JsonToMessageConverter{}, req.body);
                auto response = std::visit<nlohmann::json>(
                        Overload{
                                [](auto) {
                                    return krapi::ErrorRsp{}.to_json();
                                },
                                [&](const krapi::DiscoverNodesMsg &) {
                                    spdlog::info("Recivied node discovery message");
                                    // TODO: instead of simply sending out a static list the Discover
                                    //  server should keep track of avaliable nodes
                                    return krapi::NodesDiscoveryRsp{config.node_hosts}.to_json();
                                },
                                [&](const krapi::DiscoverTxPoolsMsg &) {
                                    spdlog::info("Recivied pool discovery message");
                                    // TODO: instead of simply sending out a static list the Discover server
                                    //  should keep track of avaliable pools
                                    return krapi::TxPoolDiscoveryRsp{config.pool_hosts}.to_json();
                                }
                        }, message
                );
                spdlog::info("Sending {}", response.dump());
                res.set_content(response.dump(), "application/json");
            }
    );
    server.listen(config.server_host, config.server_port);
}
