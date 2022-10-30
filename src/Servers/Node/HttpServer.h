//
// Created by mythi on 26/10/22.
//

#ifndef SHARED_MODELS_HTTPSERVER_H
#define SHARED_MODELS_HTTPSERVER_H

#include <utility>

#include "nlohmann/json.hpp"
#include "eventpp/eventdispatcher.h"
#include "eventpp/utilities/counterremover.h"
#include "HttpMessage.h"
#include "Transaction.h"
#include "MessageQueue.h"
#include "TransactionQueue.h"
#include "spdlog/spdlog.h"
#include "IdentityManager.h"
#include "ParsingUtils.h"
#include "ixwebsocket/IXHttpServer.h"

namespace krapi {

    class HttpServer {
        enum class HttpServerInternalMessage {
            Start,
            Block,
            Stop
        };
        using HttpServerInternalMessageQueue = eventpp::EventDispatcher<HttpServerInternalMessage, void()>;

        ServerHost m_host;
        std::shared_ptr<IdentityManager> m_identity_manager;
        HttpServerInternalMessageQueue m_internal_queue;
        MessageQueuePtr m_node_message_queue;
        TransactionQueuePtr m_tx_queue;
        ix::HttpServer m_server;


        void setup_listeners() {


            m_internal_queue.appendListener(HttpServerInternalMessage::Start, [this]() {
                auto res = m_server.listen();
                if (!res.first) {
                    spdlog::error("{}", res.second);
                    exit(1);
                }
                m_server.start();
                spdlog::info("Started http server");
            });

            m_internal_queue.appendListener(HttpServerInternalMessage::Block, [this]() {
                m_server.wait();
            });

            m_internal_queue.appendListener(HttpServerInternalMessage::Stop, [this]() {
                m_server.stop();
            });

            m_server.setOnConnectionCallback(
                    [this](
                            const ix::HttpRequestPtr &req,
                            const std::shared_ptr<ix::ConnectionState> &state
                    ) -> ix::HttpResponsePtr {
                        if (req->method == "POST") {
                            auto msg_json = nlohmann::json::parse(req->body);
                            auto msg = msg_json.get<krapi::HttpMessage>();
                            if (msg.type == krapi::NodeHttpMessageType::AddTx) {
                                spdlog::info("HttpServer recieved addtx message");
                                auto tx = msg.content.get<krapi::Transaction>();
                                m_tx_queue->dispatch(0, tx);
                            }
                            return std::make_shared<ix::HttpResponse>();
                        } else {
                            auto header = ix::WebSocketHttpHeaders();
                            header["Content-Type"] = "application/json";
                            return std::make_shared<ix::HttpResponse>(
                                    200,
                                    "OK",
                                    ix::HttpErrorCode::Ok,
                                    header,
                                    nlohmann::json(m_identity_manager->identity()).dump()
                            );
                        }

                    }
            );
        }

    public:

        explicit HttpServer(
                ServerHost host,
                MessageQueuePtr mq,
                TransactionQueuePtr tq,
                std::shared_ptr<IdentityManager> identity_manager
        ) :
                m_host(std::move(host)),
                m_node_message_queue(std::move(mq)),
                m_tx_queue(std::move(tq)),
                m_identity_manager(std::move(identity_manager)),
                m_server(m_host.second, m_host.first) {

            setup_listeners();
        }

        void start() {

            m_internal_queue.dispatch(HttpServerInternalMessage::Start);
        }

        void wait() {

            m_internal_queue.dispatch(HttpServerInternalMessage::Block);
        }

        void stop() {

            m_internal_queue.dispatch(HttpServerInternalMessage::Stop);
        }

        ~HttpServer() {

            stop();

        }
    };

} // krapi

#endif //SHARED_MODELS_HTTPSERVER_H
