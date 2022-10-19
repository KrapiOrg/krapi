//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_SERVER_H
#define KRAPI_MODELS_SERVER_H


#include "ixwebsocket/IXWebSocketServer.h"
#include "eventpp/eventqueue.h"
#include "NodeMessage.h"

namespace krapi {

    class NodeServer {
        std::shared_ptr<eventpp::EventQueue<NodeMessageType, void(const NodeMessage &)>> m_eq;
        std::jthread m_thread;
        std::string m_uri;

        void server_loop();

    public:
        explicit NodeServer(
                std::string uri,
                std::shared_ptr<eventpp::EventQueue<NodeMessageType, void(const NodeMessage &)>> eq
        );

        void start();

        void wait();

        void stop();
    };

}

#endif //KRAPI_MODELS_SERVER_H
