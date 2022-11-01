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

        // InternalMessageQueue
        m_imq.appendListener(InternalMessage::Start, [this]() {
            spdlog::info("Asking for identity of {}:{}", m_host.first, m_host.second);

            // The http port is 100 greater tham the ws port
            auto url = fmt::format("http://{}:{}", m_host.first, m_host.second + 100);
            ix::HttpResponsePtr response;
            int retryCount = 0;
            while (!response) {
                auto temp = m_http_client->get(url, m_http_client->createRequest());

                if (temp->statusCode != 200) {
                    spdlog::error("Failed to connect to {}:{}, Retrying...", m_host.first, m_host.second);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000 + 500 * retryCount));
                    retryCount++;
                } else {

                    response = temp;
                }
            }
            auto res = m_http_client->get(url, m_http_client->createRequest());
            auto body_json = nlohmann::json::parse(res->body);
            m_identity = body_json.get<int>();

            spdlog::info("Connected to Node with Identity {}", m_identity);

        });

        m_imq.appendListener(InternalMessage::Block, [this]() {
            m_ws->stop();
            m_ws->run();
        });

        m_imq.appendListener(InternalMessage::Stop, [this]() {
            m_ws->disableAutomaticReconnection();
            m_ws->stop();
        });


        // ConnectionMessageQueue
        m_eq->appendListener(
                NodeMessageType::BroadcastTx,
                [this](const NodeMessage &msg) {
                    if (msg.receiver_identity == m_identity) {
                        spdlog::info("Forwarding broadcast message to {}", m_identity);
                        auto bmsg = msg;
                        bmsg.blacklist.push_back(m_host);
                        auto bmsg_json = nlohmann::json(bmsg);
                        m_ws->send(bmsg_json.dump());
                    }
                }
        );


    }


    void NetworkConnectionManager::onMessage(const ix::WebSocketMessagePtr &message) {

        if (message->type == ix::WebSocketMessageType::Open) {
            spdlog::info("Opened connection to {}", m_ws->getUrl());
        } else if (message->type == ix::WebSocketMessageType::Error) {
            spdlog::error("{}, {}", m_ws->getUrl(), message->errorInfo.reason);
        } else if (message->type == ix::WebSocketMessageType::Message) {
            auto msg = nlohmann::json::parse(message->str).get<NodeMessage>();
            m_eq->dispatch(msg.type, msg);
        }
    }


    void NetworkConnectionManager::wait() {

        m_imq.dispatch(InternalMessage::Block);

    }

    void NetworkConnectionManager::start() {

        m_imq.dispatch(InternalMessage::Start);
    }

    void NetworkConnectionManager::stop() {

        m_imq.dispatch(InternalMessage::Stop);
    }

    int NetworkConnectionManager::identity() {

        return m_identity;
    }

    NetworkConnectionManager::~NetworkConnectionManager() {

        stop();
    }


}