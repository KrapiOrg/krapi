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
            m_peer_connection(std::make_shared<rtc::PeerConnection>(rtc::Configuration())),
            m_identity(std::move(identity)),
            m_initialized(false) {


        m_peer_connection->onLocalCandidate(std::bind_front(&PeerConnection::on_local_candidate, this));
        m_peer_connection->onLocalDescription(std::bind_front(&PeerConnection::on_local_description, this));
        m_peer_connection->onDataChannel(std::bind_front(&PeerConnection::on_data_channel, this));
    }

    PeerConnection::PeerConnection(
            NotNull<EventQueue *> event_queue,
            NotNull<SignalingClient *> signaling_client,
            std::string identity,
            std::string description
    ) :
            m_signaling_client(signaling_client),
            m_event_queue(event_queue),
            m_peer_connection(std::make_shared<rtc::PeerConnection>(rtc::Configuration())),
            m_identity(std::move(identity)),
            m_initialized(false) {

        m_peer_connection->onLocalCandidate(std::bind_front(&PeerConnection::on_local_candidate, this));
        m_peer_connection->onLocalDescription(std::bind_front(&PeerConnection::on_local_description, this));
        m_peer_connection->onDataChannel(std::bind_front(&PeerConnection::on_data_channel, this));
        m_peer_connection->setRemoteDescription(description);

    }

    RTCDataChannelResult PeerConnection::wait_for_open(RTCDataChannel datachannel) {

        co_return datachannel;
    }


    concurrencpp::result<void> PeerConnection::initialize_channel(RTCDataChannel datachannel) {

        m_initialized = true;
        spdlog::info("Waiting for DataChannel with {} to open", m_identity);
        m_datachannel = co_await wait_for_open(std::move(datachannel));
        spdlog::info("Datachannel with {} is open", m_identity);
        m_datachannel->onMessage(
                [this](rtc::message_variant rtc_message) {
                    auto message_str = std::get<std::string>(std::move(rtc_message));
                    auto message_json = nlohmann::json::parse(message_str);
                    auto peer_message = PeerMessage::from_json(message_json);

                    m_event_queue->enqueue(peer_message->type(), peer_message);
                }
        );

        m_datachannel->onClosed(
                [this]() {
                    spdlog::info("DataChannel with {} is closed", m_identity);
                }
        );

    }

    concurrencpp::result<void> PeerConnection::create_datachannel() {

        m_datachannel = m_peer_connection->createDataChannel("krapi");

        co_await initialize_channel(m_datachannel);
    }

    concurrencpp::result<Event> PeerConnection::send(Box<PeerMessage> message) {

        auto aw = m_event_queue->create_awaitable(message->tag());
        m_datachannel->send(message->to_string());
        return aw;
    }

    concurrencpp::result<PeerState> PeerConnection::request_state() {

        auto response = co_await send(
                PeerMessage::create(
                        PeerMessageType::PeerStateRequest,
                        m_signaling_client->identity(),
                        m_identity,
                        PeerMessage::create_tag()
                )
        );
        co_return response.get<PeerMessage>()->content().get<PeerState>();
    }

    concurrencpp::result<PeerType> PeerConnection::request_type() {

        auto response = co_await send(
                PeerMessage::create(
                        PeerMessageType::PeerTypeRequest,
                        m_signaling_client->identity(),
                        m_identity,
                        PeerMessage::create_tag()
                )
        );
        co_return response.get<PeerMessage>()->content().get<PeerType>();
    }

    void PeerConnection::set_remote_description(std::string description) {


        m_peer_connection->setRemoteDescription(description);
    }

    void PeerConnection::add_remote_candidate(std::string candidate) {

        m_peer_connection->addRemoteCandidate(std::move(candidate));
    }

    void PeerConnection::on_local_candidate(rtc::Candidate candidate) {

        m_signaling_client->send_and_forget(
                SignalingMessage::create(
                        SignalingMessageType::RTCCandidate,
                        m_signaling_client->identity(),
                        m_identity,
                        SignalingMessage::create_tag(),
                        std::string(std::move(candidate))
                )
        );
    }

    void PeerConnection::on_local_description(rtc::Description description) {

        m_signaling_client->send_and_forget(
                SignalingMessage::create(
                        SignalingMessageType::RTCSetup,
                        m_signaling_client->identity(),
                        m_identity,
                        SignalingMessage::create_tag(),
                        std::string(std::move(description))
                )
        );
    }

    void PeerConnection::on_data_channel(RTCDataChannel datachannel) {

        initialize_channel(std::move(datachannel)).wait();
        spdlog::info("OnDataChannel: Initialized");
    }

} // krapi