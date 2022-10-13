//
// Created by mythi on 13/10/22.
//

#include "NetworkNode.h"
#include "fmt/format.h"
#include "NetworkMessageHandeler.h"

namespace krapi {

    NetworkNode::NetworkNode(std::string uri) : m_uri(std::move(uri)) {

        socket.setUrl(fmt::format("ws://{}", m_uri));
        socket.setOnMessageCallback(NetworkMessageHandeler{socket});
    }

    void NetworkNode::start() {

        socket.start();
    }
} // krapi