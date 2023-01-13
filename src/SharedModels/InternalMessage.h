//
// Created by mythi on 24/12/22.
//

#pragma once

#include "Concepts.h"
#include "Helpers.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "uuid.h"

#include "Box.h"
#include "PeerMessage.h"
#include "SignalingMessage.h"
#include <cstdint>
#include <utility>

namespace krapi {
  enum class InternalMessageType {
    SendSignalingMessage,
    SendPeerMessage,
    RequestTypeOf,
    RequestStateOf,
    WaitForDataChannelOpen
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(
    InternalMessageType,
    {
      {InternalMessageType::SendSignalingMessage, "send_signaling_message"},
      {InternalMessageType::SendPeerMessage, "send_peer_message"},
      {InternalMessageType::RequestTypeOf, "request_type_of"},
      {InternalMessageType::RequestStateOf, "request_state_of"},
      {InternalMessageType::WaitForDataChannelOpen, "wait_for_datachannel_open"},
    }
  )

  inline std::string to_string(InternalMessageType type) {

    switch (type) {
      case InternalMessageType::SendSignalingMessage:
        return "send_signaling_message";
      case InternalMessageType::SendPeerMessage:
        return "send_peer_message";
      case InternalMessageType::RequestTypeOf:
        return "request_type_of";
      case InternalMessageType::RequestStateOf:
        return "request_state_of";
      case InternalMessageType::WaitForDataChannelOpen:
        return "wait_for_datachannel_open";
    }
  }

  template<typename T>
  concept InternalMessageContentConcept = requires(T) {
    std::is_same_v<T, SignalingMessage> || std::is_same_v<T, PeerMessage>;
    ConvertableToJson<T>;
  };

  template<InternalMessageContentConcept T>
  class InternalMessage {

   public:
    template<typename... ContentArgs>
    explicit InternalMessage(
      InternalMessageType type,
      ContentArgs &&...content_args
    )
        : m_type(type),
          m_content(T::create(std::forward<ContentArgs>(content_args)...)),
          m_timestamp(get_krapi_timestamp()) {
    }

    explicit InternalMessage(
      InternalMessageType type,
      Box<T> content
    )
        : m_type(type),
          m_content(std::move(content)),
          m_timestamp(get_krapi_timestamp()) {
    }

    explicit InternalMessage(
      InternalMessageType type,
      Box<T> content,
      uint64_t timestamp
    )
        : m_type(type),
          m_content(std::move(content)),
          m_timestamp(timestamp) {
    }


    explicit InternalMessage(InternalMessageType type)
        : m_type(type),
          m_timestamp(get_krapi_timestamp()) {
    }

    template<typename... Args>
    static Box<InternalMessage> create(Args &&...content_args) {

      return make_box<InternalMessage>(std::forward<Args>(content_args)...);
    }

    std::string tag() {
      return m_content->tag();
    }

    [[nodiscard]] InternalMessageType type() const {
      return m_type;
    }

    [[nodiscard]] Box<T> content() const {
      return m_content;
    }

    nlohmann::json content_as_json() const {

      if (!m_content) return {};
      return std::visit([](auto &&m) { return m->to_json(); }, *m_content);
    }

    std::optional<std::string> content_as_string() const {

      if (!m_content) return {};
      return std::visit([](auto &&m) { return m->to_string(); }, *m_content);
    }

    uint64_t timestamp() const {
      return m_timestamp;
    }

    nlohmann::json to_json() const {
      return {
        {"type", nlohmann::json(m_type)},
        {"content", m_content->to_json()},
        {"timestamp", m_timestamp}};
    }

    int retry_count() const {

      return m_retry_count;
    }

    void increament_retry_count() const {
      m_retry_count++;
    }

    void reset_retry_count() const {
      m_retry_count = 0;
    }

    static InternalMessage<T> from_json(nlohmann::json json) {
      return InternalMessage(
        json["type"].get<InternalMessageType>(),
        T::from_json(json["content"]),
        json["timestamp"].get<uint64_t>()
      );
    }

   private:
    InternalMessageType m_type;
    Box<T> m_content;
    uint64_t m_timestamp;
    mutable int m_retry_count{0};
  };
}// namespace krapi
