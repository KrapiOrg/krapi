//
// Created by mythi on 12/10/22.
//
#include "cxxopts.hpp"
#include "fmt/core.h"
#include "NodeManager.h"
#include "ParsingUtils.h"
#include "Message/DiscoverTxPoolsMsg.h"
#include "Message/DiscoverNodesMsg.h"
#include "Response/JsonToResponseConverter.h"
#include "Overload.h"
#include "Message/Message.h"
#include "httplib.h"

krapi::Response send_discovery_message(
        httplib::Client &client,
        const std::string &url,
        const krapi::Message &message
) {

    auto msg = std::visit([](auto &&x) { return x.to_string(); }, message);
    auto res = client.Post("/", msg, "application/json");

    return std::invoke(krapi::JsonToResponseConverter{}, res->body);
}

int main(int argc, const char **argv) {

    cxxopts::Options options("KrapiNode", "A node for contribuitng to the krapi chain");
    options.add_options()
            ("config",
             "Server configuration file path",
             cxxopts::value<std::string>()->default_value("config.json")
            );
    auto parsed_args = options.parse(argc, argv);
    auto config_path = parsed_args["config"].as<std::string>();

    auto config = krapi::parse_node_config_file(config_path);
    auto my_uri = fmt::format("{}:{}", config.server_host, config.server_port);
    auto url = fmt::format("http://127.0.0.1:7005");

    auto client = httplib::Client(url);

    spdlog::info("Sending node discovery request");
    auto dm_nodes_res = send_discovery_message(client, url, krapi::DiscoverNodesMsg{});
    spdlog::info("Sending txpool discovery request");
    auto tx_pools_res = send_discovery_message(client, url, krapi::DiscoverTxPoolsMsg{});

    auto retrieve_hosts = [](const krapi::Response &rsp) {
        return std::visit(
                Overload{
                        [](auto) { return std::vector<std::string>{}; },
                        [](const krapi::NodesDiscoveryRsp &rsp) {
                            return rsp.get_hosts();
                        },
                        [](const krapi::TxPoolDiscoveryRsp &rsp) {
                            return rsp.get_hosts();
                        }
                }, rsp
        );
    };

    auto node_uirs = retrieve_hosts(dm_nodes_res);
    spdlog::info("Received {} node uris", fmt::join(node_uirs, ","));

    auto tx_pools = retrieve_hosts(tx_pools_res);
    spdlog::info("Received {} txpool uris", fmt::join(tx_pools, ","));

    krapi::NodeManager manager(my_uri, node_uirs, tx_pools);

    manager.wait();
}