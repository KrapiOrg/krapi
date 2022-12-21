//
// Created by mythi on 19/12/22.
//

#include "PeerConnection.h"

#include <chrono>

using namespace std::chrono_literals;

namespace krapi {
    concurrencpp::result<std::shared_ptr<rtc::DataChannel>>
    PeerConnection::wait_for_open(std::shared_ptr<rtc::DataChannel> datachannel) {

        co_return datachannel;
    }

    void PeerConnection::setup_internal_listiners() {

        m_dispatcher.appendListener(
                PeerMessageType::PeerTypeRequest,
                [this](const PeerMessage &message) {
                    m_datachannel->send(
                            PeerMessage{
                                    PeerMessageType::PeerTypeResponse,
                                    m_signaling_client->identity(),
                                    m_identity,
                                    message.tag(),
                                    PeerType::Full
                            }.to_string()
                    );
                }
        );

        m_dispatcher.appendListener(
                PeerMessageType::PeerStateRequest,
                [this](const PeerMessage &message) {
                    m_datachannel->send(
                            PeerMessage{
                                    PeerMessageType::PeerStateResponse,
                                    m_signaling_client->identity(),
                                    m_identity,
                                    message.tag(),
                                    PeerState::Open
                            }.to_string()
                    );
                }
        );
    }

    void PeerConnection::setup_peer_connection() {

        m_peer_connection->onLocalCandidate(
                [this](const rtc::Candidate &candidate) {

                    m_signaling_client->send_async(
                            SignalingMessage{
                                    SignalingMessageType::RTCCandidate,
                                    m_signaling_client->identity(),
                                    m_identity,
                                    SignalingMessage::create_tag(),
                                    std::string(candidate)
                            }
                    );
                }
        );
        m_peer_connection->onLocalDescription(
                [this](const rtc::Description &description) {

                    m_signaling_client->send_async(
                            SignalingMessage{
                                    SignalingMessageType::RTCSetup,
                                    m_signaling_client->identity(),
                                    m_identity,
                                    SignalingMessage::create_tag(),
                                    std::string(description)
                            }
                    );
                }
        );

        m_peer_connection->onDataChannel(
                [this](std::shared_ptr<rtc::DataChannel> datachannel) {

                    initialize_channel(std::move(datachannel));
                }
        );
    }

    PeerConnection::PeerConnection(
            std::string
            identity,
            std::shared_ptr<SignalingClient> signaling_client
    ) :

            m_identity(std::move(identity)),
            m_peer_connection(std::make_shared<rtc::PeerConnection>(m_configuration)),
            m_signaling_client(std::move(signaling_client)),
            m_initialized(false) {

        setup_peer_connection();
        setup_internal_listiners();
    }

    PeerConnection::PeerConnection(
            std::string identity,
            std::shared_ptr<SignalingClient> signaling_client,
            std::string description
    ) :

            m_identity(std::move(identity)),
            m_peer_connection(std::make_shared<rtc::PeerConnection>(m_configuration)),
            m_signaling_client(std::move(signaling_client)),
            m_initialized(false) {

        setup_peer_connection();
        m_peer_connection->setRemoteDescription(description);

        setup_internal_listiners();
    }

    concurrencpp::result<void> PeerConnection::initialize_channel(std::shared_ptr<rtc::DataChannel> datachannel) {

        m_initialized = true;
        m_datachannel = co_await wait_for_open(std::move(datachannel));
        spdlog::info("Datachannel with {} is open", m_identity);
        m_datachannel->onMessage(
                [this](rtc::message_variant rtc_message) {
                    auto message_str = std::get<std::string>(std::move(rtc_message));
                    auto message_json = nlohmann::json::parse(message_str);
                    auto peer_message = PeerMessage::from_json(message_json);

                    if (m_promises.contains(peer_message.tag())) {
                        m_promises.at(peer_message.tag()).set_result(peer_message);
                    } else {
                        m_dispatcher.dispatch(peer_message.type(), peer_message);
                    }
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

    concurrencpp::result<PeerMessage> PeerConnection::send(PeerMessage message) {

        m_promises.emplace(message.tag(), concurrencpp::result_promise<PeerMessage>{});
        m_datachannel->send(message.to_string());
        co_return co_await m_promises[message.tag()].get_result();
    }

    concurrencpp::result<PeerState> PeerConnection::request_state() {

        auto response = co_await send(
                PeerMessage{
                        PeerMessageType::PeerStateRequest,
                        m_signaling_client->identity(),
                        m_identity,
                        PeerMessage::create_tag()
                }
        );
        co_return response.content().get<PeerState>();
    }

    concurrencpp::result<PeerType> PeerConnection::request_type() {

        auto response = co_await send(
                PeerMessage{
                        PeerMessageType::PeerTypeRequest,
                        m_signaling_client->identity(),
                        m_identity,
                        PeerMessage::create_tag()
                }
        );
        co_return response.content().get<PeerType>();
    }

    void PeerConnection::set_remote_description(std::string description) {


        m_peer_connection->setRemoteDescription(description);
    }

    void PeerConnection::add_remote_candidate(std::string candidate) {

        m_peer_connection->addRemoteCandidate(std::move(candidate));
    }

} // krapi