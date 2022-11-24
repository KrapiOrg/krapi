//
// Created by mythi on 12/11/22.
//

#pragma once

#include <condition_variable>
#include <future>

#include "spdlog/spdlog.h"

#include "PeerMap.h"
#include "Message.h"
#include "Response.h"
#include "PeerMessage.h"
#include "Block.h"
#include "TransactionPool.h"
#include "ixwebsocket/IXWebSocket.h"
#include "eventpp/eventdispatcher.h"
#include "Content/SetTransactionStatusContent.h"
#include "NodeManager.h"

namespace krapi {

    class LightNodeManager : public NodeManager {


    public:
        LightNodeManager();

        std::optional<int> get_random_light_node();
    };

} // krapi
