//
// Created by mythi on 24/10/22.
//

#include "HttpClient.h"

namespace krapi {
    HttpClient::HttpClient(ServerHost host) : m_host(std::move(host)) {

    }

    krapi::Response HttpClient::post(const std::string &path, const Message &message) {

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
} // krapi