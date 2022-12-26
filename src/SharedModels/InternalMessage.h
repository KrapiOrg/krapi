//
// Created by mythi on 24/12/22.
//

#pragma once

#include "nlohmann/json.hpp"
#include "uuid.h"
#include "Box.h"

namespace krapi {
    enum class InternalMessageType {
        SendSignalingMessage,
        SendPeerMessage,
        DataChannelOpened,
        WaitForDataChannelOpen,
        DataChannelClosed,
        SignalingServerClosed
    };

    inline std::string to_string(InternalMessageType type) {

        switch (type) {
            case InternalMessageType::SendSignalingMessage:
                return "send_signaling_message";
            case InternalMessageType::SendPeerMessage:
                return "send_peer_message";
            case InternalMessageType::DataChannelOpened:
                return "data_channel_opened";
            case InternalMessageType::WaitForDataChannelOpen:
                return "wait_for_datachannel_open";
            case InternalMessageType::DataChannelClosed:
                return "data_channel_closed";
            case InternalMessageType::SignalingServerClosed:
                return "signaling_server_closed";
        }
    }

    class InternalMessage {

    public:

        explicit InternalMessage(
                InternalMessageType type,
                std::string tag,
                nlohmann::json content
        ) :
                m_type(type),
                m_tag(std::move(tag)),
                m_content(std::move(content)) {
        }

        template<typename ...UU>
        static Box<InternalMessage> create(UU &&... params) {

            return make_box<InternalMessage>(std::forward<UU>(params)...);
        }

        static std::string create_tag() {

            return uuids::to_string(uuids::uuid_system_generator{}());
        }

        [[nodiscard]]
        InternalMessageType type() const {

            return m_type;
        }

        [[nodiscard]]
        std::string tag() const {

            return m_tag;
        }

        [[nodiscard]]
        nlohmann::json content() const {

            return m_content;
        }

    private:
        InternalMessageType m_type;
        std::string m_tag;
        nlohmann::json m_content;
    };
}
