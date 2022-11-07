//
// Created by mythi on 24/10/22.
//

#include "WebSocketServer.h"

namespace krapi {
    WebSocketServer::WebSocketServer(ServerHost host, TransactionPoolPtr transaction_pool) :
            m_host(std::move(host)),
            m_server(m_host.second, m_host.first),
            m_transaction_pool(std::move(transaction_pool)) {

        m_server.setOnClientMessageCallback(std::bind_front(&WebSocketServer::onMessage, this));
    }

    void WebSocketServer::onMessage(
            const std::shared_ptr<ix::ConnectionState> &state,
            ix::WebSocket &ws,
            const ix::WebSocketMessagePtr &message
    ) {


        using namespace ix;
        if (message->type == WebSocketMessageType::Open) {
            spdlog::info("WebSocketServer: {}:{} Just connected", state->getRemoteIp(), state->getRemotePort());
        } else if (message->type == WebSocketMessageType::Error) {
            spdlog::info(
                    "REASON: {}, STATUS_CODE: {}",
                    message->errorInfo.reason,
                    message->errorInfo.http_status
            );
        } else if (message->type == WebSocketMessageType::Message) {
            auto msg = NodeMessage::from_json(nlohmann::json::parse(message->str));
            if (msg.type() == NodeMessageType::AddTransactionToPool) {

                spdlog::info("WebSocketServer: Adding transaction {}", msg.content().dump());
                m_transaction_pool->add_transaction(
                        Transaction::from_json(msg.content()),
                        msg.identity_blacklist()
                );
            }
        }
    }


    void WebSocketServer::start() {

        auto res = m_server.listenAndStart();
        if (!res) {
            spdlog::error("Failed to start websocket server");
            exit(-1);
        }
    }

    void WebSocketServer::wait() {

        m_server.wait();
    }

    void WebSocketServer::stop() {

        m_server.stop();
    }


    WebSocketServer::~WebSocketServer() {

        stop();
    }

} // krapi