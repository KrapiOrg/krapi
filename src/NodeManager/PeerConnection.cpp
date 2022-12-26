//
// Created by mythi on 19/12/22.
//

#include "PeerConnection.h"

#include <chrono>

using namespace std::chrono_literals;

namespace krapi {

    PeerConnection::PeerConnection(
            NotNull<EventQueue *> event_queue,
            NotNull<SignalingClient *> signaling_client,
            std::string identity
    ) :
            m_signaling_client(signaling_client),
            m_event_queue(event_queue),
            m_subscription_remover(event_queue->internal_queue()),
            m_peer_connection(std::make_shared<rtc::PeerConnection>(rtc::Configuration())),
            m_peer_type(PeerType::Unknown),
            m_peer_state(PeerState::Unknown),
            m_identity(std::move(identity)) {


        m_peer_connection->onLocalCandidate(std::bind_front(&PeerConnection::on_local_candidate, this));
        m_peer_connection->onLocalDescription(std::bind_front(&PeerConnection::on_local_description, this));
        m_datachannel = m_peer_connection->createDataChannel("krapi");
        initialize_channel(m_datachannel);
        m_event_queue->append_listener(
                PeerMessageType::PeerStateUpdate,
                std::bind_front(&PeerConnection::on_peer_state_update, this)
        );
    }

    PeerConnection::PeerConnection(
            NotNull<EventQueue *> event_queue,
            NotNull<SignalingClient *> signaling_client,
            std::string identity,
            std::string description
    ) :
            m_signaling_client(signaling_client),
            m_event_queue(event_queue),
            m_subscription_remover(event_queue->internal_queue()),
            m_peer_connection(std::make_shared<rtc::PeerConnection>(rtc::Configuration())),
            m_peer_type(PeerType::Unknown),
            m_peer_state(PeerState::Unknown),
            m_identity(std::move(identity)) {

        m_peer_connection->onLocalCandidate(std::bind_front(&PeerConnection::on_local_candidate, this));
        m_peer_connection->onLocalDescription(std::bind_front(&PeerConnection::on_local_description, this));
        m_peer_connection->onDataChannel(std::bind_front(&PeerConnection::initialize_channel, this));
        m_peer_connection->setRemoteDescription(description);
        m_event_queue->append_listener(
                PeerMessageType::PeerStateUpdate,
                std::bind_front(&PeerConnection::on_peer_state_update, this)
        );
    }

    void PeerConnection::initialize_channel(RTCDataChannel datachannel) {

        if (m_datachannel == nullptr) {
            m_datachannel = datachannel;
        }

        m_datachannel->onMessage(
                [this](rtc::message_variant rtc_message) {
                    auto message_str = std::get<std::string>(std::move(rtc_message));
                    auto message_json = nlohmann::json::parse(message_str);
                    auto peer_message = PeerMessage::from_json(message_json);

                    m_event_queue->enqueue(peer_message->type(), peer_message);
                }
        );

        m_datachannel->onOpen(
                [this]() {

                    m_event_queue->enqueue(
                            InternalMessageType::DataChannelOpened,
                            InternalMessage::create(
                                    InternalMessageType::DataChannelOpened,
                                    m_identity,
                                    nlohmann::json{}
                            )
                    );
                }
        );
        m_datachannel->onClosed(
                [this]() {

                    m_event_queue->enqueue(
                            InternalMessageType::DataChannelClosed,
                            InternalMessage::create(
                                    InternalMessageType::DataChannelClosed,
                                    m_identity,
                                    nlohmann::json{}
                            )
                    );
                }
        );
    }

    bool PeerConnection::send_and_forget(Box<PeerMessage> message) const {

        if(m_datachannel == nullptr)
            return false;

        if (m_datachannel->isOpen() && !m_datachannel->isClosed()) {

            m_datachannel->send(message->to_string());
            return true;
        }
        return false;
    }

    void PeerConnection::set_remote_description(std::string description) {

        m_peer_connection->setRemoteDescription(description);
    }

    void PeerConnection::add_remote_candidate(std::string candidate) {

        m_peer_connection->addRemoteCandidate(std::move(candidate));
    }

    void PeerConnection::on_local_candidate(rtc::Candidate candidate) {

        m_event_queue->enqueue(
                InternalMessageType::SendSignalingMessage,
                InternalMessage::create(
                        InternalMessageType::SendSignalingMessage,
                        InternalMessage::create_tag(),
                        SignalingMessage(
                                SignalingMessageType::RTCCandidate,
                                m_signaling_client->identity(),
                                m_identity,
                                SignalingMessage::create_tag(),
                                std::string(std::move(candidate))
                        ).to_json()
                )
        );
    }

    void PeerConnection::on_local_description(rtc::Description description) {

        m_event_queue->enqueue(
                InternalMessageType::SendSignalingMessage,
                InternalMessage::create(
                        InternalMessageType::SendSignalingMessage,
                        InternalMessage::create_tag(),
                        SignalingMessage(
                                SignalingMessageType::RTCSetup,
                                m_signaling_client->identity(),
                                m_identity,
                                SignalingMessage::create_tag(),
                                std::string(std::move(description))
                        ).to_json()
                )
        );
    }

    concurrencpp::result<PeerState> PeerConnection::state() {

        if (m_peer_state == PeerState::Unknown) {

            auto tag = PeerMessage::create_tag();
            m_event_queue->enqueue(
                    InternalMessageType::SendPeerMessage,
                    InternalMessage::create(
                            InternalMessageType::SendPeerMessage,
                            InternalMessage::create_tag(),
                            PeerMessage(
                                    PeerMessageType::PeerStateRequest,
                                    m_signaling_client->identity(),
                                    m_identity,
                                    tag,
                                    nlohmann::json()
                            ).to_json()
                    )
            );
            auto result = co_await m_event_queue->create_awaitable(tag);
            m_peer_state = result.get<PeerMessage>()->content().get<PeerState>();
        }

        co_return m_peer_state;
    }

    concurrencpp::result<PeerType> PeerConnection::type() {

        if (m_peer_type == PeerType::Unknown) {

            auto tag = PeerMessage::create_tag();
            m_event_queue->enqueue(
                    InternalMessageType::SendPeerMessage,
                    InternalMessage::create(
                            InternalMessageType::SendPeerMessage,
                            InternalMessage::create_tag(),
                            PeerMessage(
                                    PeerMessageType::PeerTypeRequest,
                                    m_signaling_client->identity(),
                                    m_identity,
                                    tag,
                                    nlohmann::json()
                            ).to_json()
                    )
            );
            auto result = co_await m_event_queue->create_awaitable(tag);
            m_peer_type = result.get<PeerMessage>()->content().get<PeerType>();
        }

        co_return m_peer_type;
    }

    void PeerConnection::on_peer_state_update(Event event) {

        auto message = event.get<PeerMessage>();
        auto peer_id = message->sender_identity();
        if (peer_id == m_identity) {

            auto new_state = message->content().get<PeerState>();
            spdlog::info(
                    "Updated state of peer {} to {}",
                    peer_id,
                    to_string(new_state)
            );
        }
    }

    PeerConnection::~PeerConnection() {

        m_datachannel->close();
    }
} // krapi