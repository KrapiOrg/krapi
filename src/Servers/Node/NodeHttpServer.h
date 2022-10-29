//
// Created by mythi on 26/10/22.
//

#ifndef SHARED_MODELS_NODEHTTPSERVER_H
#define SHARED_MODELS_NODEHTTPSERVER_H

#include <utility>

#include "httplib.h"
#include "nlohmann/json.hpp"
#include "eventpp/eventdispatcher.h"
#include "eventpp/utilities/counterremover.h"
#include "NodeHttpMessage.h"
#include "Transaction.h"
#include "NodeMessageQueue.h"
#include "TransactionQueue.h"
#include "spdlog/spdlog.h"

namespace krapi {

    class NodeHttpServer {
        enum class HttpServerInternalMessage {
            Start,
            Stop
        };
        using HttpServerInternalMessageQueue = eventpp::EventDispatcher<HttpServerInternalMessage, void()>;

        std::jthread m_thread;
        HttpServerInternalMessageQueue m_internal_queue;
        NodeMessageQueuePtr m_node_message_queue;
        TransactionQueuePtr m_tx_queue;

        std::string m_server_host;
        int m_server_port;

        void server_loop() {

            httplib::Server server;

            m_internal_queue.appendListener(HttpServerInternalMessage::Stop, [&]() {
                server.stop();
            });

            server.Post("/", [this](const httplib::Request &req, httplib::Response &res) {
                spdlog::info("POST REQ: {}", req.body);
                auto msg_json = nlohmann::json::parse(req.body);
                auto msg = msg_json.get<NodeHttpMessage>();
                if (msg.type == NodeHttpMessageType::AddTx) {
                    spdlog::info("HttpServer recieved addtx message");
                    auto tx = msg.content.get<Transaction>();
                    m_tx_queue->dispatch(0, tx);
                }
            });
            spdlog::info("Starting http server on {}:{}", m_server_host, m_server_port);
            server.listen(m_server_host, m_server_port);
        }

        void setup_internal_queue() {

            m_internal_queue.appendListener(HttpServerInternalMessage::Start, [this]() {

                m_thread = std::jthread(&NodeHttpServer::server_loop, this);
            });

            m_internal_queue.appendListener(HttpServerInternalMessage::Stop, [this]() {
                if (m_thread.joinable()) {
                    m_thread.request_stop();
                    m_thread.join();
                }
            });
        }

    public:

        explicit NodeHttpServer(
                std::string server_host,
                int server_port,
                NodeMessageQueuePtr mq,
                TransactionQueuePtr tq
        ) :
                m_server_host(std::move(server_host)),
                m_server_port(server_port),
                m_node_message_queue(std::move(mq)),
                m_tx_queue(std::move(tq)) {

            setup_internal_queue();
            start();

        }

        void start() {

            m_internal_queue.dispatch(HttpServerInternalMessage::Start);
        }

        ~NodeHttpServer() {

            m_internal_queue.dispatch(HttpServerInternalMessage::Stop);

        }
    };

} // krapi

#endif //SHARED_MODELS_NODEHTTPSERVER_H
