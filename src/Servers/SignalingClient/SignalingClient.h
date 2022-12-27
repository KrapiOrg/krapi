//
// Created by mythi on 02/12/22.
//

#pragma once

#include "rtc/websocket.hpp"
#include "eventpp/utilities/scopedremover.h"
#include "spdlog/spdlog.h"
#include "EventQueue.h"
#include "NotNull.h"

namespace krapi {

    using RTCMessageCallback = std::function<void(Box<SignalingMessage>)>;

    class SignalingClient {

    public:

        /*!
         * Constructor for a signaling client
         */
        explicit SignalingClient(NotNull<EventQueue *> event_queue);

        /*!
         * Getter for identity
         * @return identity for the current connection to the SignalingSever
         */

        [[nodiscard]]
        std::string identity() const noexcept;

        /*!
         * Sends a message to the SignalingServer.
         * @param message Message to be sent
         * @return result containing the response for the message request
         */
        [[nodiscard]]
        concurrencpp::shared_result<Event> send(Box<SignalingMessage> message) const;

        /*!
         * Sends a message to the SignalingServer without the ability to block
         * @param message Message to be sent
         */
        void send_and_forget(Box<SignalingMessage> message) const;


        [[nodiscard]]
        concurrencpp::result<void> initialize();

        /*!
         * Sends an AvailablePeersRequest to the signaling server
         * @return A results that completes when the signaling server replies with
         * the appropriate response
         */
        [[nodiscard]]
        concurrencpp::shared_result<Event> available_peers() const;

        template<typename ...UU>
        [[nodiscard]]
        static inline std::unique_ptr<SignalingClient> create(UU &&...uu) {

            return std::make_unique<SignalingClient>(std::forward<UU>(uu)...);
        }

    private:

        /*!
         * A helper to wait for the connection to the signaling server to open.
         * @return a result that completes when the underlying rtc::WebSocket's connection is open.
         */
        [[nodiscard]]
        concurrencpp::result<void> for_open() const;

        /*!
         * A helper that sends the identity acquired during construction to the signaling server
         */
        [[nodiscard]]
        concurrencpp::shared_result<std::string> request_identity() const;

        NotNull<EventQueue *> m_event_queue;
        std::unique_ptr<rtc::WebSocket> m_ws;
        std::string m_identity;
        eventpp::ScopedRemover<EventQueueType> m_subscription_remover;
    };

    using SignalingClientPtr = std::unique_ptr<SignalingClient>;
} // krapi
