//
// Created by mythi on 02/12/22.
//

#pragma once

#include "rtc/websocket.hpp"
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
        concurrencpp::result<Event> send(Box<SignalingMessage> message);

        /*!
         * Sends a message to the SignalingServer without the ability to block
         * @param message Message to be sent
         */
        void send_and_forget(Box<SignalingMessage> message);

        /*!
         * Initializes the connection to the signaling server.
         * Does the following in sequence
         * 1. Waits for the connection with the signaling server to open.
         * 2. Waits for the identity request to arrive from the server.
         * 3. Responds with the identity acquired during construction.
         * 4. Sets the message handler.
         * @return A result that completes when the connection is initialized.
         * @pre Must not be called more than once.
         */
        concurrencpp::result<void> initialize();

    private:

        /*!
         * A helper to wait for the connection to the signaling server to open.
         * @return a result that completes when the underlying rtc::WebSocket's connection is open.
         */
        concurrencpp::result<void> wait_for_open();

        /*!
         * A helper that waits for the identity request to arrive from the signaling server
         */
        concurrencpp::result<void> wait_for_identity_request();

        /*!
         * A helper that sends the identity acquired during construction to the signaling server
         */
        concurrencpp::result<void> send_identity();

        NotNull<EventQueue *>m_event_queue;
        std::unique_ptr<rtc::WebSocket> m_ws;
        std::string m_identity;
        bool m_initialized;
    };

} // krapi
