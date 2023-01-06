//
// Created by mythi on 27/12/22.
//

#pragma once

#include <string>
#include <variant>

#include "Box.h"
#include "uuid.h"

namespace krapi {

  class Block;
  class Transaction;

  enum class InternalNotificationType {
    DataChannelOpened,
    DataChannelClosed,
    SignalingServerClosed,
    BlockAccepted,
    TransactionAddedToPool,
    BlockMined
  };

  inline std::string to_string(InternalNotificationType type) {

    switch (type) {

      case InternalNotificationType::DataChannelOpened:
        return "data_channel_opened";
      case InternalNotificationType::DataChannelClosed:
        return "data_channel_closed";
      case InternalNotificationType::SignalingServerClosed:
        return "signaling_server_closed";
      case InternalNotificationType::BlockAccepted:
        return "block_accepted";
      case InternalNotificationType::TransactionAddedToPool:
        return "transaction_added_to_pool";
      case InternalNotificationType::BlockMined:
        return "block_mined";
    }
  }

  struct Empty {};

  template<typename T>
  concept InternalNotificationContentConcept = requires(T) {
    std::is_same_v<T, std::string> || std::is_same_v<T, Transaction> || std::is_same_v<T, Block>;
  };


  template<typename T>
  class InternalNotification {

    InternalNotificationType m_type;
    T m_content;
    std::string m_tag;

   public:
    InternalNotification(InternalNotificationType type)
      requires(std::is_same_v<T, Empty>)
        : m_type(type), m_tag(create_tag()) {}

    InternalNotification(InternalNotificationType type, std::string tag)
      requires(std::is_same_v<T, Empty>)
        : m_type(type), m_tag(std::move(tag)) {}

    InternalNotification(
      InternalNotificationType type,
      T content,
      std::string tag
    )
      requires(InternalNotificationContentConcept<T>)
        : m_type(type), m_content(std::move(content)), m_tag(std::move(tag)) {}

    InternalNotification(InternalNotificationType type, T content)
      requires(InternalNotificationContentConcept<T>)
        : m_type(type), m_content(std::move(content)), m_tag(create_tag()) {}

    static std::string create_tag() {

      return uuids::to_string(uuids::uuid_system_generator{}());
    }

    template<typename... U>
    static Box<InternalNotification> create(U &&...args) {

      return make_box<InternalNotification>(std::forward<U>(args)...);
    }

    [[nodiscard]] std::string tag() const noexcept { return m_tag; }

    [[nodiscard]] InternalNotificationType type() const noexcept {

      return m_type;
    }

    [[nodiscard]] T content() const noexcept { return m_content; }
  };

  template<>
  class InternalNotification<void> : public InternalNotification<Empty> {

   public:
    using InternalNotification<Empty>::InternalNotification;
  };
}// namespace krapi