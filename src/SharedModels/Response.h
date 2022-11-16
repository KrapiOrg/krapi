//
// Created by mythi on 20/10/22.
//

#pragma once

#include "nlohmann/json.hpp"

namespace krapi {
    enum class ResponseType {
        PeerAvailable,
        AvailablePeers,
        PeerIdentity,
        RTCSetup
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(ResponseType, {

        { ResponseType::PeerAvailable, "peer_avaliable" },
        { ResponseType::AvailablePeers, "available_peers" },
        { ResponseType::PeerIdentity, "peer_identity" },
        { ResponseType::RTCSetup, "rtc_setup" }
    })

    struct Response {
        ResponseType type;
        nlohmann::json content;

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type",    type},
                    {"content", content}
            };
        }

        [[nodiscard]]
        std::string to_string() const {

            return to_json().dump();
        }

        static Response from_json(const std::string &json_str) {

            auto json = nlohmann::json::parse(json_str);
            return from_json(json);
        }

        static Response from_json(nlohmann::json json) {

            return Response{
                    json["type"].get<ResponseType>(),
                    json["content"]
            };
        }
    };
}
