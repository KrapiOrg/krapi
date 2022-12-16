//
// Created by mythi on 17/12/22.
//

#pragma once

#include <functional>
#include "spdlog/spdlog.h"
#include "rtc/websocketserver.hpp"
#include "concurrencpp/concurrencpp.h"

#include "SignalingMessage.h"

namespace krapi {

    /*!
     * A simple wrapper around an rtc::WebSocket instance provided when
     * The signaling server gets a new connection
     */
    class SignalingSocket {

        std::function<void(SignalingMessage)> m_on_message;
        std::function<void(std::string)> m_on_disconnected;
        std::shared_ptr<rtc::WebSocket> m_socket;
        std::string m_identity;
        bool m_initialized;

        /*!
         * A helper that waits for a particular websocket connection to open.
         * @return a result that is completed once the connection is opened.
         */
        concurrencpp::result<void> wait_for_open();

        /*!
         * A helper that is used to send a PeerMessage::IdentityRequest to the client and
         * waits for the response.
         * @return A result that will contain the identity of the of client
         */
        concurrencpp::result<std::string> wait_for_identity();

        /*!
         * A helper that is used to signal to the client that its identity has been acknowledged
         */
        void send_identity_acknowledgement();

    public:
        /*!
         * Constructor for SignalingSocket.
         * @param socket the socket to wrap around.
         */
        explicit SignalingSocket(
                std::shared_ptr<rtc::WebSocket> socket
        );

        /*!
         * Initialization method, does the following in sequence.
         * 1. Waits for the underlying rtc::WebSocket instance to open.
         *
         * 2. Waits until the client sends its identity.
         *
         * 3. Sends an acknowledgement when the newly provided identity is verified.
         *
         * 4. Sets the handlers for rtc::WebSocket::onMessage and rtc::WebSocket::onClosed.
         *
         * @param on_message handler for when a message is available.
         * @param on_closed handler for when the underlying rtc::WebSocket instance closes.
         * @return The identity of the client acquired by performing the previous steps.
         */
        concurrencpp::result<std::string> initialize(
                std::function<void(SignalingMessage)> on_message,
                std::function<void(std::string)> on_closed
        );

        /*!
         * Takes a SignalingMessage, converts it to a string and sends it over the socket.
         * @param message Message to be sent.
         * @pre SignalingSocket::initialize() has to have been called.
         */
        void send(const SignalingMessage &message);

        /*!
         * Getter for the identity of this socket
         * @return std::string containing the identity that was acquired during initialization.
         */
        [[nodiscard]]
        std::string identity() const;

        /// This class is not copyable , movable or assignable.

        SignalingSocket(const SignalingSocket &) = delete;

        SignalingSocket(SignalingSocket &&) = delete;

        SignalingSocket &operator=(const SignalingSocket &) = delete;
    };

}