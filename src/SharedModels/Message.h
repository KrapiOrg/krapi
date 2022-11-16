//
// Created by mythi on 20/10/22.
//

#include "nlohmann/json.hpp"

namespace krapi {
    enum class MessageType {
        GetAvailablePeers,
        GetIdentitiy,
        RTCSetup,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(MessageType, {
        { MessageType::GetAvailablePeers, "get_available_peers" },
        { MessageType::GetIdentitiy, "get_identity" },
        { MessageType::RTCSetup, "rtc_setup" },
    })

    struct Message {
        MessageType type;
        nlohmann::json content;

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type",    type},
                    {"content", content}
            };
        }

        operator std::string() {
            return to_string();
        }

        [[nodiscard]]
        std::string to_string() const {

            return to_json().dump();
        }

        static Message from_json(nlohmann::json json) {

            return Message{
                    json["type"].get<MessageType>(),
                    json["content"].get<nlohmann::json>()
            };
        }
    };
}
