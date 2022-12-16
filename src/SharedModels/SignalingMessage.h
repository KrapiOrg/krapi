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
        Acknowledgement,
        SetIdentityRequest,
        PeerAvailable,
        RTCSetup,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(SignalingMessageType, {
        { SignalingMessageType::AvailablePeersRequest, "available_peers_request" },
        { SignalingMessageType::AvailablePeersResponse, "available_peers_response" },
        { SignalingMessageType::IdentityRequest, "identity_request" },
        { SignalingMessageType::IdentityResponse, "identity_response" },
        { SignalingMessageType::Acknowledgement, "acknowledgement" },
        { SignalingMessageType::SetIdentityRequest, "set_identity_request" },
        { SignalingMessageType::IdentityResponse, "identity_response" },
        { SignalingMessageType::PeerAvailable, "peer_available" },
        { SignalingMessageType::RTCSetup, "rtc_setup" },
    })

    class SignalingMessage {

    public:

        explicit SignalingMessage(
                SignalingMessageType type,
                std::string sender_identity,
                std::string tag,
                nlohmann::json content = {}
        ) :
                m_type(type),
                m_sender_identity(std::move(sender_identity)),
                m_tag(std::move(tag)),
                m_content(std::move(content)) {

        }

        [[nodiscard]]
        SignalingMessageType type() const {

            return m_type;
        }

        [[nodiscard]]
        std::string sender_identity() const {

            return m_sender_identity;
        }

        [[nodiscard]]
        std::string tag() const {

            return m_tag;
        }

        [[nodiscard]]
        nlohmann::json content() const {

            return m_content;
        }

        static std::string create_tag() {

            return uuids::to_string(uuids::uuid_system_generator{}());
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"type",     m_type},
                    {"sender_identity", m_sender_identity},
                    {"tag",      m_tag},
                    {"content",  m_content}
            };
        }

        [[nodiscard]]
        std::string to_string() const {

            return to_json().dump();
        }

        static SignalingMessage from_json(nlohmann::json json) {

            return SignalingMessage{
                    json["type"].get<SignalingMessageType>(),
                    json["sender_identity"].get<std::string>(),
                    json["tag"].get<std::string>(),
                    json["content"].get<nlohmann::json>()
            };
        }

    private:
        SignalingMessageType m_type;
        std::string m_sender_identity;
        std::string m_tag;
        nlohmann::json m_content;
    };
}
