#pragma once

#include "SignalingMessage.h"
#include "rtc/websocketserver.hpp"
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace krapi {

  /*!
    * A simple wrapper around an rtc::WebSocketServer that keeps track of its clients
    * and blocks the thread its running on.
    */
  using WebSocket = std::shared_ptr<rtc::WebSocket>;
  using RawWebSocket = rtc::WebSocket *;

  class SignalingServer {
    std::unordered_set<WebSocket> m_sockets;

    std::unordered_map<RawWebSocket, std::string> m_socket_identity_map;
    mutable std::recursive_mutex m_mutex;
    std::atomic<bool> m_blocking_bool;
    rtc::WebSocketServer m_server;

    void on_client_message(
      RawWebSocket socket,
      rtc::message_variant message
    );

    void on_client_closed(RawWebSocket);

   public:
    explicit SignalingServer();


    [[nodiscard]] int port() const;


    ~SignalingServer();
  };

}// namespace krapi
