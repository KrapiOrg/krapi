//
// Created by mythi on 12/11/22.
//

#ifndef SHARED_MODELS_P2PNODEMANAGER_H
#define SHARED_MODELS_P2PNODEMANAGER_H

#include <condition_variable>
#include <future>

#include "spdlog/spdlog.h"
#include "eventpp/eventdispatcher.h"
#include "nlohmann/json.hpp"
#include "ixwebsocket/IXWebSocketMessage.h"
#include "ixwebsocket/IXWebSocket.h"

#include "PeerMap.h"
#include "Message.h"
#include "Response.h"
#include "PeerMessage.h"
#include "Blockchain/Block.h"
#include "Blockchain/TransactionPool.h"

namespace krapi {

    class P2PNodeManager {
    public:
        enum class Event {
            TransactionReceived,
            BlockReceived,
        };
    private:
        using TxEventDispatcher = eventpp::EventDispatcher<Event, void(Transaction)>;
        using BlockEventDispatcher = eventpp::EventDispatcher<Event, void(Block)>;

        TxEventDispatcher m_tx_dispatcher;
        BlockEventDispatcher m_block_dispatcher;

        PeerMap peer_map;
        std::mutex blocking_mutex;
        std::condition_variable blocking_cv;

        rtc::Configuration rtc_config;
        ix::WebSocket ws;
        int my_id;

        TransactionPool m_transaction_pool;

        std::shared_ptr<rtc::PeerConnection> create_connection(int);

        void onWsResponse(const Response &);

        void onRemoteMessage(int id, const PeerMessage &);


    public:

        P2PNodeManager();

        void wait();

        void broadcast_message(const PeerMessage &);

        void append_listener(Event event, std::function<void(Block)> listener);

        void append_listener(Event event, std::function<void(Transaction)> listener);

        [[nodiscard]]
        int id() const;
    };

} // krapi

#endif //SHARED_MODELS_P2PNODEMANAGER_H
