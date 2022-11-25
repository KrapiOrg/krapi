//
// Created by mythi on 23/11/22.
//

#pragma once

#include <future>
#include <chrono>
#include "eventpp/eventdispatcher.h"
#include "spdlog/spdlog.h"
#include "rtc/datachannel.hpp"
#include "PeerMessage.h"

using namespace std::chrono_literals;

namespace krapi {

    using PeerMessageCallback = std::function<void(PeerMessage)>;

    class KrapiRTCDataChannel {
    public:
        enum class Event {
            Open,
            Close,
            Message
        };
    private:
        eventpp::EventDispatcher<Event, void()> m_void_dispatcher;
        eventpp::EventDispatcher<Event, void(PeerMessage)> m_message_dispatcher;
        std::shared_ptr<rtc::DataChannel> m_channel;
        std::unordered_map<std::string, std::promise<PeerMessage>> m_tagged_messages;
        std::unordered_map<std::string, std::optional<PeerMessageCallback>> m_callbacks;

    public:

        explicit KrapiRTCDataChannel(
                std::shared_ptr<rtc::DataChannel> channel
        ) : m_channel(std::move(channel)) {

            m_channel->onMessage(
                    [this](rtc::message_variant rtc_message) {
                        auto message_str = std::get<std::string>(rtc_message);
                        auto message_json = nlohmann::json::parse(message_str);
                        auto message = PeerMessage::from_json(message_json);
                        auto tag = message.tag();

                        if (m_tagged_messages.contains(tag)) {

                            m_tagged_messages[tag].set_value(message);

                            if (m_callbacks[tag].has_value()) {

                                m_callbacks[tag].value()(message);
                            }

                        }
                        m_message_dispatcher.dispatch(Event::Message, message);
                        m_tagged_messages.erase(tag);
                    }

            );

            m_channel->onOpen([this]() {

                m_void_dispatcher.dispatch(Event::Open);
            });
            m_channel->onClosed([this]() {

                m_void_dispatcher.dispatch(Event::Close);
            });
        }

        // Returns the message tag and a future handle
        std::future<PeerMessage> send(
                PeerMessage message,
                std::optional<PeerMessageCallback> callback = std::nullopt
        ) {

            if (m_channel->isOpen()) {

                std::string tag = message.tag();

                m_tagged_messages[tag] = std::promise<PeerMessage>{};

                m_channel->send(std::move(message));

                m_callbacks[tag] = std::move(callback);
                return m_tagged_messages[tag].get_future();
            }
            return {};
        }

        [[nodiscard]]
        std::shared_ptr<rtc::DataChannel> raw_channel() const {

            return m_channel;
        }

        [[nodiscard]]
        bool is_open() const {

            return m_channel->isOpen();
        }

        void append_listener(Event event, const std::function<void(PeerMessage)> &callback) {

            m_message_dispatcher.appendListener(event, callback);
        }

        void append_listener(Event event, const std::function<void()> &callback) {

            m_void_dispatcher.appendListener(event, callback);
        }
    };

} // krapi
