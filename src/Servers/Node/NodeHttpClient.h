//
// Created by mythi on 24/10/22.
//

#ifndef SHARED_MODELS_NODEHTTPCLIENT_H
#define SHARED_MODELS_NODEHTTPCLIENT_H

#include "httplib.h"
#include "Response.h"
#include "Message.h"
#include "spdlog/spdlog.h"

namespace krapi {

    class NodeHttpClient {
        httplib::Client client;
    public:
        explicit NodeHttpClient(const std::string& url): client(url) {

        }

        Response post(
                const Message &message
        ) {
            auto res = client.Post("/", to_string(nlohmann::json(message)), "application/json");
            auto res_json = nlohmann::json::parse(res->body);
            return res_json.get<krapi::Response>();
        }

    };

} // krapi

#endif //SHARED_MODELS_NODEHTTPCLIENT_H
