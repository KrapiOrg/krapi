//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_SERVER_H
#define KRAPI_MODELS_SERVER_H


#include "ixwebsocket/IXWebSocketServer.h"

namespace krapi {
    class Server {
        ix::WebSocketServer server;

    public:
        explicit Server(int port, const std::string &host);

        void start();

        void wait();
    };

}

#endif //KRAPI_MODELS_SERVER_H
