//
// Created by mythi on 13/10/22.
//


#include "cxxopts.hpp"
#include "httplib.h"

#include "ParsingUtils.h"
#include "Message.h"
#include "Response.h"

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

    auto config = krapi::parse_config<krapi::DiscoveryServerConfig>(config_path);

    spdlog::info("Starting Discovery Server on {}:{}", config.server_host, config.server_port);

    httplib::Server server;
    server.Post(
            "/",
            [&](
                    const httplib::Request &req,
                    httplib::Response &res
            ) {
                spdlog::info("Received {} from {}", req.body, req.remote_addr);
                auto message_json = nlohmann::json::parse(req.body);
                auto message = message_json.get<krapi::Message>();
                krapi::Response response;

                switch (message) {
                    case krapi::Message::Acknowledge:
                    case krapi::Message::CreateTransaction:
                        break;
                    case krapi::Message::DiscoverNodes:
                        response = krapi::Response{
                                krapi::ResponseType::NodesDiscovered,
                                config.node_hosts
                        };
                        break;
                    case krapi::Message::DiscoverTxPools:
                        response = krapi::Response{
                                krapi::ResponseType::TxPoolsDiscovered,
                                config.pool_hosts
                        };
                        break;
                    case krapi::Message::DiscoverIdentity:
                        response = krapi::Response{
                                krapi::ResponseType::IdentityDiscovered,
                                config.identity_host
                        };
                        break;

                }
                auto response_json = nlohmann::json(response);
                spdlog::info("Sending\n{} to {}", response_json.dump(4), req.remote_addr);
                res.set_content(response_json.dump(), "application/json");
            }
    );
    server.listen(config.server_host, config.server_port);
}
