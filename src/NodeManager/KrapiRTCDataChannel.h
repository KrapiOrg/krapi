//
// Created by mythi on 23/11/22.
//

#pragma once

#include <future>
#include "spdlog/spdlog.h"
#include "rtc/datachannel.hpp"
#include "PeerMessage.h"

namespace krapi {

    using PeerMessageCallback = std::function<void(PeerMessage)>;

    class KrapiRTCDataChannel {

        std::shared_ptr<rtc::DataChannel> m_channel;
        std::unordered_map<int, std::promise<PeerMessage>> m_tagged_messages;
        std::unordered_map<int, std::optional<PeerMessageCallback>> m_callbacks;
        PeerMessageCallback m_on_message;
        std::function<void()> m_on_close;

    public:

        explicit KrapiRTCDataChannel(
                std::shared_ptr<rtc::DataChannel> channel,
                PeerMessageCallback on_message,
                std::function<void()> on_close
        ) :
                m_channel(std::move(channel)),
                m_on_message(std::move(on_message)),
                m_on_close(std::move(on_close)) {

            m_channel->onMessage(
                    [this](rtc::message_variant rtc_message) {
                        auto message_str = std::get<std::string>(rtc_message);
                        auto message_json = nlohmann::json::parse(message_str);
                        auto message = PeerMessage::from_json(message_json);
                        auto tag = message.tag();

                        m_on_message(message);

                        if (m_tagged_messages.contains(tag)) {

                            m_tagged_messages[tag].set_value(message);

                            if (m_callbacks[tag]) {
                                m_callbacks[tag].value()(message);
                            }
                            m_tagged_messages.erase(tag);
                            m_callbacks.erase(tag);
                        }


                    }
            );
        }


        // Returns the message tag and a future handle
        std::shared_future<PeerMessage> send(
                PeerMessage message,
                std::optional<PeerMessageCallback> callback = std::nullopt
        ) {

            int tag = message.tag();
            m_tagged_messages[tag] = std::promise<PeerMessage>{};

            m_channel->send(std::move(message));

            m_callbacks[tag] = std::move(callback);
            return m_tagged_messages[tag].get_future();
        }

        [[nodiscard]]
        std::shared_ptr<rtc::DataChannel> raw_channel() const {

            return m_channel;
        }

        [[nodiscard]]
        bool is_open() const {

            return m_channel->isOpen();
        }
    };

} // krapi
