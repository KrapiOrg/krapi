//
// Created by mythi on 13/10/22.
//

#include <future>
#include <thread>
#include "NodeManager.h"
#include "spdlog/spdlog.h"
#include "Response.h"


namespace krapi {
    using namespace eventpp;

    NodeManager::NodeManager(
            std::string my_uri,
            std::string identity_uri,
            std::vector<std::string> network_hosts
    )
            : m_my_uri(std::move(my_uri)),
              m_identity(-1),
              m_network_node_hosts(std::move(network_hosts)),
              m_eq(std::make_shared<EventDispatcher<NodeMessageType, void(const NodeMessage &)>>()) {


        identity_socket.setUrl(fmt::format("ws://{}", identity_uri));
        identity_socket.setOnMessageCallback(
                [this](const ix::WebSocketMessagePtr &msg) {
                    if (msg->type == ix::WebSocketMessageType::Message) {
                        auto resp_json = nlohmann::json::parse(msg->str);
                        auto resp = resp_json.get<Response>();
                        if (resp.type == ResponseType::IdentityFound) {
                            auto identity = resp.content.get<int>();
                            spdlog::info("Assigned Identity {}", identity);

                            identity_promise.set_value(identity);
                        }
                    }
                }
        );

        identity_socket.start();
        auto identity_future = std::shared_future(identity_promise.get_future());
        identity_future.wait();
        m_identity = identity_future.get();
        assert(m_identity != -1);
    }

    void NodeManager::connect_to_nodes() {

        for (const auto &uri: m_network_node_hosts) {
            if (uri != m_my_uri) {
                m_nodes.emplace_back(uri, m_eq);
            }
        }
    }

    void NodeManager::wait() {

        for (auto &server: m_nodes) {
            server.wait();
        }
    }


    int NodeManager::identity() {

        return m_identity;
    }

    void NodeManager::setup_listeners() {

    }

    NodeManager::~NodeManager() {

        for (auto &node: m_nodes) {
            node.stop();
        }
        identity_socket.disableAutomaticReconnection();
        identity_socket.stop();
        spdlog::info("Successfully terminated");
    }


} // krapi