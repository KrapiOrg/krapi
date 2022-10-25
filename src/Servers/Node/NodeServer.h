//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_SERVER_H
#define KRAPI_MODELS_SERVER_H

#include <future>

#include "ixwebsocket/IXWebSocketServer.h"
#include "NodeMessage.h"
#include "NodeMessageQueue.h"

namespace krapi {

    class NodeServer {
        NodeMessageQueuePtr m_eq;

        std::promise<int> identity_promise;
        int m_identity;

        std::jthread m_thread;

        std::string m_uri;

        void server_loop();

    public:
        explicit NodeServer(
                std::string uri,
                NodeMessageQueuePtr eq
        );

        void wait();

        void stop();

        int identity();
    };

}

#endif //KRAPI_MODELS_SERVER_H
