//
// Created by mythi on 13/10/22.
//

#include "NetworkConnectionManager.h"

#include <utility>
#include "spdlog/spdlog.h"

namespace krapi {

    NetworkConnectionManager::NetworkConnectionManager(
            ServerHost host,
            MessageQueuePtr eq
    ) : m_host(std::move(host)),
        m_eq(std::move(eq)),
        m_ws(std::make_shared<ix::WebSocket>()),
        m_http_client(std::make_shared<ix::HttpClient>()),
        m_identity(-1) {

        setup_listeners();

        spdlog::info("Trying to connect to {}:{}", m_host.first, m_host.second);

        m_ws->setUrl(fmt::format("ws://{}:{}", m_host.first, m_host.second));
        m_ws->setOnMessageCallback(std::bind_front(&NetworkConnectionManager::onMessage, this));
    }

    void NetworkConnectionManager::setup_listeners() {

        // ConnectionMessageQueue
        m_eq->appendListener(
                NodeMessageType::AddTransactionToPool,
                [this](const NodeMessage &msg) {
                    if (msg.receiver_identity() == m_identity) {
                        spdlog::info(
                                "NetworkConnectionManager: Forwarding transaction {} to {}",
                                msg.content().dump(),
                                m_identity
                        );
                        m_ws->send(msg.to_json().dump());
                    }
                }
        );

    }

    void NetworkConnectionManager::onMessage(const ix::WebSocketMessagePtr &message) {

        if (message->type == ix::WebSocketMessageType::Open) {
            spdlog::info("NetworkConnectionManager: Connection established with {}", m_ws->getUrl());
        } else if (message->type == ix::WebSocketMessageType::Error) {
            spdlog::error("{}, {}", m_ws->getUrl(), message->errorInfo.reason);
        } else if (message->type == ix::WebSocketMessageType::Message) {

            auto msg = NodeMessage::from_json(nlohmann::json::parse(message->str));
            m_eq->dispatch(msg.type(), msg);
        }
    }


    void NetworkConnectionManager::wait() {

        m_ws->stop();
        m_ws->run();

    }

    void NetworkConnectionManager::start() {

        spdlog::info("NetworkConnectionManager: Asking for identity of {}:{}", m_host.first, m_host.second);

        auto identity_response = request(HttpMessage{NodeHttpMessageType::RequestIdentity});
        if (identity_response.type == NodeHttpMessageType::IdentityResponse) {

            m_identity = identity_response.content.get<int>();
            spdlog::info("NetworkConnectionManager: Received identity {}", m_identity);
        }
        spdlog::info("NetworkConnectionManager: Starting WebSocket connection", m_identity);
        m_ws->start();
        spdlog::info("NetworkConnectionManager: Connected to Node with Identity {}", m_identity);
    }

    void NetworkConnectionManager::stop() {

        m_ws->disableAutomaticReconnection();
        m_ws->stop();
    }

    int NetworkConnectionManager::identity() {

        return m_identity;
    }

    NetworkConnectionManager::~NetworkConnectionManager() {

        stop();
    }

    HttpMessage NetworkConnectionManager::request(
            const HttpMessage &message
    ) {

        // The http port is 100 greater than the ws port

        static const auto url = fmt::format("http://{}:{}", m_host.first, m_host.second + 100);
        ix::HttpResponsePtr response;
        int retryCount = 0;

        while (!response) {
            auto temp = m_http_client->post(url, message.to_json().dump(), m_http_client->createRequest());

            if (temp->statusCode != 200) {
                spdlog::error(
                        "NetworkConnectionManager: Failed to connect to {}:{}, Retrying...",
                        m_host.first,
                        m_host.second
                );
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 + 500 * retryCount));
                retryCount++;
            } else {

                response = temp;
            }
        }
        return HttpMessage::fromJson(nlohmann::json::parse(response->body));
    }


}