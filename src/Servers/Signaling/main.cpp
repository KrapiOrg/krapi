//
// Created by mythi on 09/11/22.
//

#include "SignalingServer.h"

int main(int argc, char **argv) {

    krapi::SignalingServer server;
    spdlog::info("Started server on 127.0.0.1:{}", server.port());
}