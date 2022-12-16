//
// Created by mythi on 17/12/22.
//

#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

#include "SignalingSocket.h"

namespace krapi {

    /*!
    * A simple wrapper around an rtc::WebSocketServer that keeps track of its clients
    * and blocks the thread its running on.
    */
    class SignalingServer {

        std::unordered_map<std::string, std::shared_ptr<SignalingSocket>> m_sockets;
        mutable std::recursive_mutex m_mutex;
        std::atomic<bool> m_blocking_bool;
        rtc::WebSocketServer m_server;

        /*!
         * Thread safe helper that fetches the socket for a specific connection
         * @param identity the identity for the client whose socket to be fetched
         * @return A weak_ptr to the SignalingSocket, nullptr if the socket is not present
         */
        std::weak_ptr<SignalingSocket> get_socket(const std::string &identity) const;

        /*!
         * A helper that fetches the identities for the sockets currently open
         * @param identity_to_filter_for
         * @return A vector containing all identities for the currently connected clients except for identity_to_filter_for
         */
        std::vector<std::string> get_identities(std::string_view identity_to_filter_for) const;


        /*!
         * Handler for Signaling messages received from any currently open connection
         * @param message
         */
        void on_client_message(const SignalingMessage &message) const;

        /*!
         * For cleanup when a socket is closed
         * @param identity
         */
        void on_client_closed(const std::string &identity);

    public:

        /*!
         * Constructor for SignalingServer
         */
        explicit SignalingServer();

        /*!
         * Getter for the port this server is running on
         * @return
         */
        [[nodiscard]]
        int port() const;

        /*!
         * Destructor for this class, blocks until a certain condition is satisfied and calls
         * stop() on the underlying rtc::WebSocketServer instance
         */
        ~SignalingServer();

    };

} // krapi
