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
    SignalingServerOpened,
    SignalingServerClosed,
    TransactionAddedToPool,
    BlockMined,
    MinedBlockValidated
  };

  inline std::string to_string(InternalNotificationType type) {

    switch (type) {

      case InternalNotificationType::DataChannelOpened:
        return "data_channel_opened";
      case InternalNotificationType::DataChannelClosed:
        return "data_channel_closed";
      case InternalNotificationType::SignalingServerOpened:
        return "signaling_server_opened";
      case InternalNotificationType::SignalingServerClosed:
        return "signaling_server_closed";
      case InternalNotificationType::TransactionAddedToPool:
        return "transaction_added_to_pool";
      case InternalNotificationType::BlockMined:
        return "block_mined";
      case InternalNotificationType::MinedBlockValidated:
        return "mined_block_validated";
    }
  }

  struct Empty {};

  template<typename T>
  concept ContentHasCreate = requires(T) {
    std::is_same_v<T, Transaction> || std::is_same_v<T, Block>;
  };

  template<typename T>
  concept ContentHasNoCreate = requires(T) {
    std::is_same_v<T, std::string>;
  };


  template<typename T>
  class InternalNotification {

    InternalNotificationType m_type;
    T m_content;
    std::string m_tag;

   public:
    InternalNotification(InternalNotificationType type)
      requires(std::is_same_v<T, Empty>)
        : m_type(type), m_tag(create_tag()) {
    }

    InternalNotification(InternalNotificationType type, std::string tag)
      requires(std::is_same_v<T, Empty>)
        : m_type(type), m_tag(std::move(tag)) {
    }


    InternalNotification(
      InternalNotificationType type,
      T content,
      std::string tag
    )
      requires(ContentHasNoCreate<T>)
        : m_type(type),
          m_content(std::move(content)),
          m_tag(std::move(tag)) {
    }

    InternalNotification(
      InternalNotificationType type,
      T content,
      std::string tag
    )
      requires(ContentHasCreate<T>)
        : m_type(type),
          m_content(std::move(content)),
          m_tag(std::move(tag)) {
    }

    InternalNotification(
      InternalNotificationType type,
      T content
    )
      requires(ContentHasCreate<T>)
        : m_type(type),
          m_content(std::move(content)),
          m_tag(create_tag()) {
    }

    template<typename... ContentArgs>
    InternalNotification(
      InternalNotificationType type,
      std::string tag,
      ContentArgs &&...content_args
    )
      requires(ContentHasCreate<T>)
        : m_type(type),
          m_tag(std::move(tag)),
          m_content(T::create(std::forward<ContentArgs>(content_args)...)) {
    }

    template<typename... ContentArgs>
    InternalNotification(
      InternalNotificationType type,
      ContentArgs &&...content_args
    )
      requires(ContentHasCreate<T>)
        : m_type(type),
          m_tag(create_tag()),
          m_content(T::create(std::forward<ContentArgs>(content_args)...)) {
    }

    static std::string create_tag() {

      return uuids::to_string(uuids::uuid_system_generator{}());
    }

    template<typename... U>
    static Box<InternalNotification> create(U &&...args) {

      return make_box<InternalNotification>(std::forward<U>(args)...);
    }

    [[nodiscard]] std::string tag() const noexcept {
      return m_tag;
    }

    [[nodiscard]] InternalNotificationType type() const noexcept {

      return m_type;
    }

    [[nodiscard]] T content() const noexcept {
      return m_content;
    }
  };

  template<>
  class InternalNotification<void> : public InternalNotification<Empty> {

   public:
    using InternalNotification<Empty>::InternalNotification;
  };
}// namespace krapi