//
// Created by mythi on 24/10/22.
//

#ifndef NODE_NODEHTTPCLIENT_H
#define NODE_NODEHTTPCLIENT_H

#include "Response.h"
#include "Message.h"
#include "spdlog/spdlog.h"
#include "ixwebsocket/IXHttpClient.h"
#include "ParsingUtils.h"

namespace krapi {

    class HttpClient {
        ix::HttpClient m_client;
        ServerHost m_host;
    public:
        explicit HttpClient(ServerHost host) : m_host(std::move(host)) {

        }

        krapi::Response post(
                const std::string &path,
                const krapi::Message &message
        ) {

            auto url = fmt::format("http://{}:{}{}", m_host.first, m_host.second, path);
            auto message_json = nlohmann::json(message);
            auto res = m_client.post(
                    url,
                    to_string(message_json),
                    m_client.createRequest(url, ix::HttpClient::kPost)
            );
            auto res_json = nlohmann::json::parse(res->body);
            return res_json.get<krapi::Response>();
        }

    };

} // krapi

#endif //NODE_NODEHTTPCLIENT_H
