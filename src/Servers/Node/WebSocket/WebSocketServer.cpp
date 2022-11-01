//
// Created by mythi on 24/10/22.
//

#include "WebSocketServer.h"

namespace krapi {
    WebSocketServer::WebSocketServer(ServerHost host, TransactionQueuePtr txq) :
            m_host(std::move(host)),
            m_server(m_host.second, m_host.first),
            m_txq(std::move(txq)) {

        setup_listeners();
    }

    void WebSocketServer::start() {

        m_internal_queue.dispatch(WsServerInternalMessage::Start);
    }

    void WebSocketServer::wait() {

        m_internal_queue.dispatch(WsServerInternalMessage::Block);
    }

    void WebSocketServer::stop() {

        m_internal_queue.dispatch(WsServerInternalMessage::Stop);
    }

    void WebSocketServer::setup_listeners() {

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

    WebSocketServer::~WebSocketServer() {

        stop();
    }
} // krapi