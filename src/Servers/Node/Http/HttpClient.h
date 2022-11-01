//
// Created by mythi on 24/10/22.
//

#ifndef NODE_NODEHTTPCLIENT_H
#define NODE_NODEHTTPCLIENT_H

#include "ixwebsocket/IXHttpClient.h"
#include "spdlog/spdlog.h"

#include "Response.h"
#include "Message.h"
#include "ParsingUtils.h"

namespace krapi {

    class HttpClient {
        ix::HttpClient m_client;
        ServerHost m_host;
    public:
        explicit HttpClient(ServerHost host);

        krapi::Response post(
                const std::string &path,
                const krapi::Message &message
        );

    };

} // krapi

#endif //NODE_NODEHTTPCLIENT_H
