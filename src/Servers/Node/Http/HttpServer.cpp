//
// Created by mythi on 26/10/22.
//

#include "HttpServer.h"

namespace krapi {
    HttpServer::HttpServer(
            ServerHost host,
            MessageQueuePtr mq,
            TransactionPoolPtr tq,
            std::shared_ptr<IdentityManager> identity_manager
    ) :
            m_host(std::move(host)),
            m_node_message_queue(std::move(mq)),
            m_transaction_pool(std::move(tq)),
            m_identity_manager(std::move(identity_manager)),
            m_server(m_host.second, m_host.first) {

        setup_listeners();
    }

    void HttpServer::start() {

        m_internal_queue.dispatch(InternalMessage::Start);
    }

    void HttpServer::wait() {

        m_internal_queue.dispatch(InternalMessage::Block);
    }

    void HttpServer::stop() {

        m_internal_queue.dispatch(InternalMessage::Stop);
    }

    HttpServer::~HttpServer() {

        stop();

    }

    void HttpServer::setup_listeners() {


        m_internal_queue.appendListener(InternalMessage::Start, [this]() {
            auto res = m_server.listen();
            if (!res.first) {
                spdlog::error("{}", res.second);
                exit(1);
            }
            m_server.start();
            spdlog::info("Started http server");
        });

        m_internal_queue.appendListener(InternalMessage::Block, [this]() {
            m_server.wait();
        });

        m_internal_queue.appendListener(InternalMessage::Stop, [this]() {
            m_server.stop();
        });

        m_server.setOnConnectionCallback(
                [this](
                        const ix::HttpRequestPtr &req,
                        const std::shared_ptr<ix::ConnectionState> &state
                ) -> ix::HttpResponsePtr {
                    auto msg = krapi::HttpMessage::fromJson(nlohmann::json::parse(req->body));
                    krapi::HttpMessage response;

                    if (msg.type == krapi::NodeHttpMessageType::AddTx) {
                        spdlog::info("HttpServer: Adding transaction {}", msg.content.dump());
                        m_transaction_pool->add_transaction(Transaction::from_json(msg.content));
                        response = krapi::HttpMessage{
                                krapi::NodeHttpMessageType::TxAdded
                        };
                    } else if (msg.type == krapi::NodeHttpMessageType::RequestIdentity) {
                        response = krapi::HttpMessage{
                                krapi::NodeHttpMessageType::IdentityResponse,
                                m_identity_manager->identity()
                        };
                    }

                    auto header = ix::WebSocketHttpHeaders();
                    header["Content-Type"] = "application/json";

                    return std::make_shared<ix::HttpResponse>(
                            200,
                            "OK",
                            ix::HttpErrorCode::Ok,
                            header,
                            response.to_json().dump()
                    );

                }
        );
    }
} // krapi