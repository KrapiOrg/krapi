//
// Created by mythi on 24/10/22.
//

#ifndef NODE_NODEWEBSOCKETSERVER_H
#define NODE_NODEWEBSOCKETSERVER_H

#include "ixwebsocket/IXWebSocketServer.h"
#include "nlohmann/json.hpp"
#include "../Models/NodeMessage.h"
#include "spdlog/spdlog.h"
#include "../Utils/MessageQueue.h"
#include "../Utils/TransactionQueue.h"
#include "../Managers/IdentityManager.h"
#include "ParsingUtils.h"

namespace krapi {

    class WebSocketServer {
        enum class WsServerInternalMessage {
            Start,
            Block,
            Stop
        };
        using WsServerInternalMessageQueue = eventpp::EventDispatcher<WsServerInternalMessage, void()>;
        WsServerInternalMessageQueue m_internal_queue;

        ServerHost m_host;
        ix::WebSocketServer m_server;

        TransactionQueuePtr m_txq;

        void setup_listeners() {

            m_internal_queue.appendListener(WsServerInternalMessage::Start, [this]() {
                auto res = m_server.listenAndStart();
                if (!res) {
                    spdlog::error("Failed to start websocket server");
                    exit(-1);
                }
            });

            m_internal_queue.appendListener(WsServerInternalMessage::Block, [this]() {
                m_server.wait();
            });

            m_internal_queue.appendListener(WsServerInternalMessage::Stop, [this]() {
                m_server.stop();
            });

            m_server.setOnClientMessageCallback(
                    [this](
                            const std::shared_ptr<ix::ConnectionState> &state,
                            ix::WebSocket &ws,
                            const ix::WebSocketMessagePtr &message
                    ) {
                        using namespace ix;
                        if (message->type == WebSocketMessageType::Open) {
                            spdlog::info("{}:{} Just connected", state->getRemoteIp(), state->getRemotePort());
                        } else if (message->type == WebSocketMessageType::Error) {
                            spdlog::info(
                                    "REASON: {}, STATUS_CODE: {}",
                                    message->errorInfo.reason,
                                    message->errorInfo.http_status
                            );
                        } else if (message->type == WebSocketMessageType::Message) {
                            auto msg_json = nlohmann::json::parse(message->str);
                            auto msg = msg_json.get<NodeMessage>();
                            if (msg.type == NodeMessageType::BroadcastTx) {
                                auto tx = msg.content.get<Transaction>();
                                spdlog::info("Dispatching transaction {} to txq", msg.content.dump());
                                m_txq->dispatch(0, tx);
                            }
                        }
                    }
            );
        }


    public:
        explicit WebSocketServer(
                ServerHost host,
                TransactionQueuePtr txq
        ) : m_host(std::move(host)),
            m_server(m_host.second, m_host.first),
            m_txq(std::move(txq)) {
            setup_listeners();
        }

        void start() {

            m_internal_queue.dispatch(WsServerInternalMessage::Start);
        }

        void wait() {

            m_internal_queue.dispatch(WsServerInternalMessage::Block);
        }

        void stop() {

            m_internal_queue.dispatch(WsServerInternalMessage::Stop);
        }

        ~WebSocketServer() {

            stop();
        }
    };

} // krapi

#endif //NODE_NODEWEBSOCKETSERVER_H
