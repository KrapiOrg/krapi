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
        WaitForDataChannelOpen
    };

    inline std::string to_string(InternalMessageType type) {

        switch (type) {
            case InternalMessageType::SendSignalingMessage:
                return "send_signaling_message";
            case InternalMessageType::SendPeerMessage:
                return "send_peer_message";
            case InternalMessageType::WaitForDataChannelOpen:
                return "wait_for_datachannel_open";
        }
    }

    template<typename T>
    concept InternalMessageContentConcept = requires(T){
        std::is_same_v<T, SignalingMessage> || std::is_same_v<T, PeerMessage>;
    };

    template<InternalMessageContentConcept T>
    class InternalMessage {

    public:

        explicit InternalMessage(
                InternalMessageType type,
                Box<T> content
        ) :
                m_type(type),
                m_content(std::move(content)) {
        }

        explicit InternalMessage(
                InternalMessageType type
        ) :
                m_type(type) {
        }

        template<typename ...UU>
        static Box<InternalMessage> create(UU &&... params) {

            return make_box<InternalMessage>(std::forward<UU>(params)...);
        }

        std::string tag() {

            return m_content->tag();
        }

        [[nodiscard]]
        InternalMessageType type() const {

            return m_type;
        }

        [[nodiscard]]
        Box<T> content() const {

            return m_content;
        }

        nlohmann::json content_as_json() const {

            if (!m_content)
                return {};
            return std::visit([](auto &&m) { return m->to_json(); }, *m_content);
        }

        std::optional<std::string> content_as_string() const {

            if (!m_content)
                return {};
            return std::visit([](auto &&m) { return m->to_string(); }, *m_content);
        }

    private:
        InternalMessageType m_type;
        Box<T> m_content;
    };
}
