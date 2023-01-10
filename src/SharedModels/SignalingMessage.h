//
// Created by mythi on 20/10/22.
//

#pragma once

#include "Box.h"
#include "nlohmann/json.hpp"
#include "uuid.h"

namespace krapi {
  enum class SignalingMessageType {
    DEFAULT,
    AvailablePeersRequest,
    AvailablePeersResponse,
    IdentityRequest,
    IdentityResponse,
    Acknowledgement,
    PeerAvailable,
    RTCSetup,
    RTCCandidate,
    PeerClosed
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(
    SignalingMessageType,
    {{SignalingMessageType::DEFAULT, "default"},
     {SignalingMessageType::AvailablePeersRequest, "available_peers_request"},
     {SignalingMessageType::AvailablePeersResponse, "available_peers_response"},
     {SignalingMessageType::IdentityRequest, "identity_request"},
     {SignalingMessageType::IdentityResponse, "identity_response"},
     {SignalingMessageType::PeerAvailable, "peer_available"},
     {SignalingMessageType::RTCSetup, "rtc_setup"},
     {SignalingMessageType::RTCCandidate, "rtc_candidate"},
     {SignalingMessageType::PeerClosed, "peer_closed"}}
  )

  inline std::string to_string(SignalingMessageType type) {

    switch (type) {
      case SignalingMessageType::DEFAULT:
        return "default";
      case SignalingMessageType::AvailablePeersRequest:
        return "available_peers_request";
      case SignalingMessageType::AvailablePeersResponse:
        return "available_peers_response";
      case SignalingMessageType::IdentityRequest:
        return "identity_request";
      case SignalingMessageType::IdentityResponse:
        return "identity_response";
      case SignalingMessageType::Acknowledgement:
        return "acknowledgement";
      case SignalingMessageType::PeerAvailable:
        return "peer_available";
      case SignalingMessageType::RTCSetup:
        return "rtc_setup";
      case SignalingMessageType::RTCCandidate:
        return "rtc_candidate";
      case SignalingMessageType::PeerClosed:
        return "peer_closed";
    }
  }

  class SignalingMessage {

   public:
    explicit SignalingMessage(
      SignalingMessageType type,
      std::string receiver_identity,
      std::string tag,
      nlohmann::json content = {}
    )
        : m_type(type),
          m_receiver_identity(std::move(receiver_identity)),
          m_tag(std::move(tag)), m_content(std::move(content)) {
    }
    explicit SignalingMessage(
      SignalingMessageType type,
      std::string sender_identity,
      std::string receiver_identity,
      std::string tag,
      nlohmann::json content = {}
    )
        : m_type(type), m_sender_identity(std::move(sender_identity)),
          m_receiver_identity(std::move(receiver_identity)),
          m_tag(std::move(tag)), m_content(std::move(content)) {
    }

    template<typename... UU>
    static Box<SignalingMessage> create(UU &&...params) {

      return make_box<SignalingMessage>(std::forward<UU>(params)...);
    }

    [[nodiscard]] SignalingMessageType type() const {
      return m_type;
    }

    [[nodiscard]] std::string sender_identity() const {

      return m_sender_identity;
    }

    [[nodiscard]] std::string receiver_identity() const {

      return m_receiver_identity;
    }

    [[nodiscard]] std::string tag() const { return m_tag; }

    [[nodiscard]] nlohmann::json content() const { return m_content; }

    static std::string create_tag() {

      return uuids::to_string(uuids::uuid_system_generator{}());
    }

    [[nodiscard]] nlohmann::json to_json() const {

      return {
        {"type", m_type},
        {"sender_identity", m_sender_identity},
        {"receiver_identity", m_receiver_identity},
        {"tag", m_tag},
        {"content", m_content}};
    }

    [[nodiscard]] std::string to_string() const { return to_json().dump(); }

    static Box<SignalingMessage> from_json(nlohmann::json json) {

      return make_box<SignalingMessage>(
        json["type"].get<SignalingMessageType>(),
        json["sender_identity"].get<std::string>(),
        json["receiver_identity"].get<std::string>(),
        json["tag"].get<std::string>(),
        json["content"].get<nlohmann::json>()
      );
    }

    void set_sender_identity(std::string identity) {
      m_sender_identity = identity;
    }

   private:
    SignalingMessageType m_type;
    std::string m_sender_identity;
    std::string m_receiver_identity;
    std::string m_tag;
    nlohmann::json m_content;
  };
}// namespace krapi
