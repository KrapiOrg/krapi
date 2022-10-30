//
// Created by mythi on 13/10/22.
//


#include "cxxopts.hpp"
#include "ixwebsocket/IXHttpServer.h"

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

    spdlog::info("Starting Discovery Server on {}:{}", config.discovery_host.first, config.discovery_host.second);

    ix::HttpServer server(config.discovery_host.second, config.discovery_host.first);
    server.setOnConnectionCallback(
            [&](
                    const ix::HttpRequestPtr &req,
                    const std::shared_ptr<ix::ConnectionState> &state
            ) -> ix::HttpResponsePtr {

                if (req->method == "POST") {
                    if (req->uri == "/") {
                        spdlog::info("Received {} from {}:{}", req->body, state->getRemoteIp(), state->getRemotePort());
                        auto message_json = nlohmann::json::parse(req->body);
                        auto message = message_json.get<krapi::Message>();
                        krapi::Response response;

                        switch (message) {
                            case krapi::Message::Acknowledge:
                            case krapi::Message::CreateTransaction:
                                break;
                            case krapi::Message::DiscoverNodes:
                                response = krapi::Response{
                                        krapi::ResponseType::NodesDiscovered,
                                        config.network_hosts
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
                        spdlog::info("Sending\n{} to {}:{}", response_json.dump(4), state->getRemoteIp(),
                                     state->getRemotePort());
                        auto headers = ix::WebSocketHttpHeaders();
                        headers["Content-Type"] = "application/json";
                        return std::make_shared<ix::HttpResponse>(
                                200,
                                "",
                                ix::HttpErrorCode::Ok,
                                headers,
                                response_json.dump()
                        );
                    }
                }
                return std::make_shared<ix::HttpResponse>(404, "", ix::HttpErrorCode::Invalid);
            }
    );
    auto res = server.listen();
    if (!res.first) {
        spdlog::error("{}", res.second);
        exit(1);
    }
    server.start();
    server.wait();
}
