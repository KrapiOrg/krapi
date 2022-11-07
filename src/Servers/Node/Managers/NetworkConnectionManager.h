//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_SERVER_H
#define KRAPI_MODELS_SERVER_H


#include "ixwebsocket/IXWebSocketServer.h"
#include "ixwebsocket/IXHttpClient.h"
#include "eventpp/eventdispatcher.h"

#include "NodeMessage.h"
#include "MessageQueue.h"
#include "ParsingUtils.h"
#include "InternalMessageQueue.h"
#include "HttpMessage.h"

namespace krapi {

    class NetworkConnectionManager {

        ServerHost m_host;
        eventpp::EventDispatcher<NodeMessageType, void(NodeMessage)> m_event_dispatcher;

        std::shared_ptr<ix::HttpClient> m_http_client;
        std::shared_ptr<ix::WebSocket> m_ws;

        int m_identity;

        void onMessage(const ix::WebSocketMessagePtr &message);


        HttpMessage request(
                const HttpMessage &message
        );

    public:
        explicit NetworkConnectionManager(ServerHost host);

        void start();

        void stop();

        int identity();

        void send(const NodeMessage &message);

        void add_listener(NodeMessageType type, std::function<void(NodeMessage)> listener);

        ~NetworkConnectionManager();
    };

}

#endif //KRAPI_MODELS_SERVER_H
