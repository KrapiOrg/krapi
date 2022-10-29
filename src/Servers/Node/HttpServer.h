//
// Created by mythi on 26/10/22.
//

#ifndef SHARED_MODELS_HTTPSERVER_H
#define SHARED_MODELS_HTTPSERVER_H

#include <utility>

#include "httplib.h"
#include "nlohmann/json.hpp"
#include "eventpp/eventdispatcher.h"
#include "eventpp/utilities/counterremover.h"
#include "HttpMessage.h"
#include "Transaction.h"
#include "MessageQueue.h"
#include "TransactionQueue.h"
#include "spdlog/spdlog.h"

namespace krapi {

    class HttpServer {
        enum class HttpServerInternalMessage {
            Start,
            Stop
        };
        using HttpServerInternalMessageQueue = eventpp::EventDispatcher<HttpServerInternalMessage, void()>;

        std::jthread m_thread;
        HttpServerInternalMessageQueue m_internal_queue;
        MessageQueuePtr m_node_message_queue;
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
                auto msg = msg_json.get<HttpMessage>();
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

                m_thread = std::jthread(&HttpServer::server_loop, this);
            });

            m_internal_queue.appendListener(HttpServerInternalMessage::Stop, [this]() {
                if (m_thread.joinable()) {
                    m_thread.request_stop();
                    m_thread.join();
                }
            });
        }

    public:

        explicit HttpServer(
                std::string server_host,
                int server_port,
                MessageQueuePtr mq,
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

        ~HttpServer() {

            m_internal_queue.dispatch(HttpServerInternalMessage::Stop);

        }
    };

} // krapi

#endif //SHARED_MODELS_HTTPSERVER_H
