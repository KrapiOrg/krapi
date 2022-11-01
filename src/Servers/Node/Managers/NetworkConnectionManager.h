//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_SERVER_H
#define KRAPI_MODELS_SERVER_H

#include <future>

#include "ixwebsocket/IXWebSocketServer.h"
#include "eventpp/eventdispatcher.h"
#include "../Models/NodeMessage.h"
#include "../Utils/MessageQueue.h"
#include "ParsingUtils.h"
#include "ixwebsocket/IXHttpClient.h"

namespace krapi {

    class NetworkConnectionManager {
        enum class InternalMessage {
            Start,
            Block,
            Stop
        };
        using InternalMessageQueue = eventpp::EventDispatcher<InternalMessage, void()>;

        ServerHost m_host;
        MessageQueuePtr m_eq;
        InternalMessageQueue m_imq;

        std::shared_ptr<ix::HttpClient> m_http_client;
        std::shared_ptr<ix::WebSocket> m_ws;

        int m_identity;


        void onMessage(const ix::WebSocketMessagePtr &message);

        void setup_listeners();

    public:
        explicit NetworkConnectionManager(
                ServerHost host,
                MessageQueuePtr eq
        );

        void start();

        void wait();

        void stop();

        int identity();

        ~NetworkConnectionManager();
    };

}

#endif //KRAPI_MODELS_SERVER_H
