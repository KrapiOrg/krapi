//
// Created by mythi on 24/10/22.
//

#ifndef NODE_NODEIDENTITYMANAGER_H
#define NODE_NODEIDENTITYMANAGER_H

#include <future>
#include "ixwebsocket/IXWebSocket.h"
#include "nlohmann/json.hpp"
#include "fmt/format.h"
#include "Response.h"
#include "spdlog/spdlog.h"
#include "ParsingUtils.h"

namespace krapi {

    class IdentityManager {
        int m_identity;

        std::promise<int> identity_promise;

        ServerHost m_host;
        ix::WebSocket ws;

        void onMessage(const ix::WebSocketMessagePtr &msg) {

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

    public:
        explicit IdentityManager(ServerHost host) : m_host(std::move(host)), m_identity{-1} {

            ws.setUrl(fmt::format("ws://{}:{}", m_host.first, m_host.second));
            ws.setOnMessageCallback([this](auto &&msg) { onMessage(std::forward<decltype(msg)>(msg)); });

            ws.start();
            auto identity_future = std::shared_future(identity_promise.get_future());
            identity_future.wait();
            m_identity = identity_future.get();
            assert(m_identity != -1);
        }

        [[nodiscard]]
        int identity() const {

            return m_identity;
        }

        ~IdentityManager() {

            ws.disableAutomaticReconnection();
            ws.stop();
        }
    };

} // krapi

#endif //NODE_NODEIDENTITYMANAGER_H
