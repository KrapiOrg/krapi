//
// Created by mythi on 20/10/22.
//

#pragma once

#include "nlohmann/json.hpp"

namespace krapi {
    enum class SignalingMessageType {
        AvailablePeersRequest,
        AvailablePeersResponse,
        IdentityRequest,
        IdentityResponse,
        PeerAvailable,
        RTCSetup,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(SignalingMessageType, {
        { SignalingMessageType::AvailablePeersRequest, "available_peers_request" },
        { SignalingMessageType::AvailablePeersResponse, "available_peers_response" },
        { SignalingMessageType::IdentityRequest, "identity_request" },
        { SignalingMessageType::IdentityResponse, "identity_response" },
        { SignalingMessageType::PeerAvailable, "peer_available" },
        { SignalingMessageType::RTCSetup, "rtc_setup" },
    })

    struct SignalingMessage {
        SignalingMessageType type;
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

        static SignalingMessage from_json(nlohmann::json json) {

            return SignalingMessage{
                    json["type"].get<SignalingMessageType>(),
                    json["content"].get<nlohmann::json>()
            };
        }
    };
}
