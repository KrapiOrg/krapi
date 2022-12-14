//
// Created by mythi on 02/12/22.
//

#pragma once

#include <future>
#include <variant>
#include "SignalingMessage.h"
#include "ixwebsocket/IXWebSocket.h"
#include "spdlog/spdlog.h"

namespace krapi {

    class SignalingClient {

        std::mutex m_mutex;

        using PromiseMap = std::map<std::string, std::promise<SignalingMessage>>;
        using RTCSetupCallback = std::function<void(SignalingMessage)>;

    public:

        SignalingClient();

        std::string get_identity();

        std::future<SignalingMessage> send(SignalingMessage message);

        std::future<SignalingMessage> send(SignalingMessageType message_type);

        void on_rtc_setup(RTCSetupCallback callback) {

            m_rtc_setup_callback = std::move(callback);
        }

    private:

        std::string m_identity;
        RTCSetupCallback m_rtc_setup_callback;
        PromiseMap m_promises;
        ix::WebSocket m_ws;
    };

} // krapi
