//
// Created by mythi on 12/10/22.
//
#include <fstream>
#include "cxxopts.hpp"
#include "ixwebsocket/IXWebSocketServer.h"
#include "fmt/core.h"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"

int main(int argc, const char **argv) {

    cxxopts::Options options("KrapiNode", "A node for contribuitng to the krapi chain");
    options.add_options()
            ("config",
             "Server configuration file path",
             cxxopts::value<std::string>()->default_value("config.json")
            );
    auto parsed_args = options.parse(argc, argv);

    int port;
    std::vector<std::string> connect_to;

    // Parse json_file supplied by args
    {
        auto config_path = parsed_args["config"].as<std::string>();
        std::ifstream config_file(config_path);

        if (!config_file.is_open()) {
            spdlog::error("Failed to open configuration file {}", config_path);
            return 1;
        }
        auto parsed_config_file = nlohmann::json::parse(config_file);
        parsed_config_file["server_port"].get_to(port);
        parsed_config_file["connect_to"].get_to(connect_to);
    }


    ix::WebSocketServer server(port, "127.0.0.1");

    spdlog::info("Launching node server on: 127.0.0.1:{}", port);
    server.setOnClientMessageCallback(
            [](
                    const std::shared_ptr<ix::ConnectionState> &connectionState,
                    ix::WebSocket &webSocket,
                    const ix::WebSocketMessagePtr &msg
            ) {
                spdlog::info("Remote ip: {}", connectionState->getRemoteIp());

                if (msg->type == ix::WebSocketMessageType::Open) {
                    spdlog::info("New connection");
                    spdlog::info("id: {}", connectionState->getId());
                    spdlog::info("Uri: {}", msg->openInfo.uri);
                    spdlog::info("Headers:");
                    for (auto it: msg->openInfo.headers) {
                        spdlog::info("\t {},{}", it.first, it.second);
                    }
                } else if (msg->type == ix::WebSocketMessageType::Message) {
                    spdlog::info("Received: {}", msg->str);
                    webSocket.send(msg->str, msg->binary);
                }
            }
    );
    auto res = server.listen();
    if (!res.first) {
        spdlog::error(res.second);
        return 1;
    }

    server.start();

    std::vector<std::shared_ptr<ix::WebSocket>> me_as_a_client_sockets;

    for (const auto &uri: connect_to) {
        auto socket = std::make_shared<ix::WebSocket>();

        socket->setUrl(fmt::format("ws://{}", uri));
        socket->setOnMessageCallback([&uri](const ix::WebSocketMessagePtr &message) {
            if (message->type == ix::WebSocketMessageType::Open) {
                spdlog::warn("Connected to server on port {}", uri);
            }
        });
        socket->start();
        me_as_a_client_sockets.push_back(socket);
    }

    server.wait();
}