//
// Created by mythi on 20/10/22.
//

#pragma once

#include "nlohmann/json.hpp"
#include "uuid.h"

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
        std::string tag;
        nlohmann::json content;

        explicit SignalingMessage(SignalingMessageType type) :
                type(type),
                tag(create_tag()) {

        }

        explicit SignalingMessage(
                SignalingMessageType type,
                nlohmann::json content
        ) :
                type(type),
                content(std::move(content)),
                tag(create_tag()) {

        }

        explicit SignalingMessage(
                SignalingMessageType type,
                std::string tag,
                nlohmann::json content
        ) :
                type(type),
                tag(std::move(tag)),
                content(std::move(content)) {

        }


        static std::string create_tag() {

            return uuids::to_string(uuids::uuid_system_generator{}());
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type",    type},
                    {"content", content},
                    {"tag",     tag}
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
                    json["tag"].get<std::string>(),
                    json["content"].get<nlohmann::json>()
            };
        }
    };
}
