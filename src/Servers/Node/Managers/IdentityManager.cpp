//
// Created by mythi on 24/10/22.
//

#include "IdentityManager.h"

namespace krapi {
    IdentityManager::IdentityManager(ServerHost host) : m_host(std::move(host)), m_identity{-1} {

        ws.setUrl(fmt::format("ws://{}:{}", m_host.first, m_host.second));
        ws.setOnMessageCallback([this](auto &&msg) { onMessage(std::forward<decltype(msg)>(msg)); });

        ws.start();
        auto identity_future = std::shared_future(identity_promise.get_future());
        identity_future.wait();
        m_identity = identity_future.get();
        assert(m_identity != -1);
    }

    int IdentityManager::identity() const {

        return m_identity;
    }

    IdentityManager::~IdentityManager() {

        ws.disableAutomaticReconnection();
        ws.stop();
    }

    void IdentityManager::onMessage(const ix::WebSocketMessagePtr &msg) {

        if (msg->type == ix::WebSocketMessageType::Message) {
            auto resp_json = nlohmann::json::parse(msg->str);
            auto resp = resp_json.get<Response>();
            if (resp.type == ResponseType::IdentityFound) {
                auto identity = resp.content.get<int>();
                spdlog::info("Got assigned the identity {}", identity);

                identity_promise.set_value(identity);
            }
        }

    }
} // krapi