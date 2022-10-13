//
// Created by mythi on 13/10/22.
//

#include "Server.h"
#include "spdlog/spdlog.h"
#include "ServerMessageHandeler.h"

namespace krapi {

    Server::Server(
            int port,
            const std::string &host
    ) : server(port, host) {

        server.setOnClientMessageCallback(ServerMessageHandeler{});
    }

    void Server::start() {

        spdlog::info("Starting node server on: {}:{}", server.getHost(), server.getPort());
        auto res = server.listen();
        if (!res.first) {
            spdlog::error(res.second);
            exit(1);
        }
        server.start();
    }

    void Server::wait() {

        server.wait();
    }

}