//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_SERVER_H
#define KRAPI_MODELS_SERVER_H

#include <future>

#include "ixwebsocket/IXWebSocketServer.h"
#include "eventpp/eventdispatcher.h"
#include "NodeMessage.h"

namespace krapi {

    class NodeServer {
        std::shared_ptr<eventpp::EventDispatcher<NodeMessageType, void(const NodeMessage &)>> m_eq;
        std::promise<int> identity_promise;
        int m_identity{-1};
        std::jthread m_thread;
        std::string m_uri;

        void server_loop();

    public:
        explicit NodeServer(
                std::string uri,
                std::shared_ptr<eventpp::EventDispatcher<NodeMessageType, void(const NodeMessage &)>> eq
        );

        void start();

        void wait();

        void stop();

        int identity();
    };

}

#endif //KRAPI_MODELS_SERVER_H
