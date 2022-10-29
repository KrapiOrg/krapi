//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_SERVER_H
#define KRAPI_MODELS_SERVER_H

#include <future>

#include "ixwebsocket/IXWebSocketServer.h"
#include "NodeMessage.h"
#include "MessageQueue.h"

namespace krapi {

    class NetworkConnection {
        MessageQueuePtr m_eq;

        std::promise<int> identity_promise;
        int m_identity;

        std::jthread m_thread;

        std::string m_uri;

        void server_loop();

    public:
        explicit NetworkConnection(
                std::string uri,
                MessageQueuePtr eq
        );

        void wait();

        void stop();

        int identity();
    };

}

#endif //KRAPI_MODELS_SERVER_H
