//
// Created by mythi on 20/10/22.
//

#ifndef NODE_NODEMESSAGE_H
#define NODE_NODEMESSAGE_H

#include "nlohmann/json.hpp"
#include <unordered_set>
#include <utility>

namespace krapi {

    enum class NodeMessageType {
        AddTransactionToPool
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(NodeMessageType, {
        { NodeMessageType::AddTransactionToPool, "add_tx_to_pool" }
    })

    class NodeMessage {
        NodeMessageType m_type;
        nlohmann::json m_content;
        int m_sender_identity;
        int m_receiver_identity;
        std::unordered_set<int> m_identity_blacklist;
    public:

        explicit NodeMessage(
                NodeMessageType type,
                nlohmann::json content,
                int sender_identity,
                int receiver_identity,
                std::unordered_set<int> identity_blacklist
        )
                : m_type(type),
                  m_content(std::move(content)),
                  m_sender_identity(sender_identity),
                  m_receiver_identity(receiver_identity),
                  m_identity_blacklist(std::move(identity_blacklist)) {

        }

        [[nodiscard]]
        NodeMessageType type() const {

            return m_type;
        }

        [[nodiscard]]
        const nlohmann::json &content() const {

            return m_content;
        }

        [[nodiscard]]
        int sender_identity() const {

            return m_sender_identity;
        }

        [[nodiscard]]
        int receiver_identity() const {

            return m_receiver_identity;
        }

        [[nodiscard]]
        const std::unordered_set<int> &identity_blacklist() const {

            return m_identity_blacklist;
        }

        static NodeMessage from_json(nlohmann::json json) {

            return NodeMessage{
                    json["type"].get<NodeMessageType>(),
                    json["content"].get<nlohmann::json>(),
                    json["sender_identity"].get<int>(),
                    json["receiver_identity"].get<int>(),
                    json["identity_blacklist"].get<std::unordered_set<int>>()
            };
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return
                    {
                            {"type",               m_type},
                            {"content",            m_content},
                            {"sender_identity",    m_sender_identity},
                            {"receiver_identity",  m_receiver_identity},
                            {"identity_blacklist", m_identity_blacklist}
                    };
        }
    };

} // krapi

#endif //NODE_NODEMESSAGE_H
