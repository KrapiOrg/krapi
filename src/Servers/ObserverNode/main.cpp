#include <iostream>
#include <stack>

#include "NodeManager.h"
#include "Content/BlockHeadersResponseContent.h"
#include "Block.h"

using namespace krapi;

static constexpr auto genesis_hash = "6FBA8017848885FB34C183BF4B6015D9C53307ABCD1F86505A271ED4B387265A";

int main() {

    const auto genesis_header = BlockHeader::from_json(
            {
                    {"hash",          genesis_hash},
                    {"merkle_root",   "0"},
                    {"nonce",         0},
                    {"previous_hash", "0"},
                    {"timestamp",     1668542625}
            }
    );

    constexpr std::array options{
            "1. Get blockchain from specific node",
            "2. Get blockchain from all nodes",
            "3. Get tip from specific node",
            "4. Get tips from all nodes"
    };

    auto manager = NodeManager{PeerType::Observer};
    auto connected_to = manager.connect_to_peers().get();
    manager.set_state(PeerState::Open);
    fmt::print("Connected to [{}]\n", fmt::join(connected_to, ","));

    auto print_chain = [&genesis_header](
            const std::vector<Block> &blocks,
            std::optional<std::string> start_from = std::nullopt
    ) -> ErrorOr<void> {
        if (blocks.empty())
            return tl::make_unexpected(KrapiErr{"Node {} has no blocks"});

        if (!start_from.has_value())
            start_from = blocks.back().hash();

        std::unordered_map<std::string, Block> block_map;
        block_map[genesis_hash] = Block{genesis_header, {}};
        std::unordered_set<std::string> visited_blocks;

        for (const auto &block: blocks) {

            block_map[block.hash()] = block;
        }
        if (!block_map.contains(start_from.value())) {
            return tl::make_unexpected(
                    KrapiErr{
                            fmt::format("Blockchain does not have the block {}", start_from.value().substr(0, 10))
                    }
            );
        }
        std::stack<std::string> stk;
        stk.push(start_from.value());
        constexpr int line_max = 4;
        int counter = line_max;
        while (!stk.empty()) {
            auto top = stk.top();
            stk.pop();
            if (visited_blocks.contains(top))
                continue;
            visited_blocks.insert(top);
            if (counter) {

                if (counter == line_max)
                    fmt::print("{}", block_map[top].contrived_hash());
                else
                    fmt::print(" <- {}", block_map[top].contrived_hash());

                counter--;
            } else {
                fmt::print(" <- {}", block_map[top].contrived_hash());
                fmt::print("\n");
                counter = line_max;
            }
            for (const auto &[hash, block]: block_map) {
                if (block.hash() == block_map[top].header().previous_hash()) {
                    stk.push(hash);
                }
            }
        }
        return {};
    };
    auto get_blocks = [&](int id, const BlockHeader &header) -> ErrorOr<std::vector<Block>> {
        auto headers_response = TRY(
                TRY(
                        manager.send(
                                id,
                                PeerMessage{
                                        PeerMessageType::BlockHeadersRequest,
                                        manager.id(),
                                        PeerMessage::create_tag(),
                                        header.to_json()
                                }
                        )
                ).get()
        );

        auto headers_content = BlockHeadersResponseContent::from_json(headers_response.content());
        auto headers = headers_content.headers();
        auto blocks = std::vector<Block>{headers.size()};
        for (const auto &block_header: headers_content.headers()) {

            auto block_response = TRY(
                    TRY(manager.send(
                                id,
                                PeerMessage{
                                        PeerMessageType::BlockRequest,
                                        manager.id(),
                                        PeerMessage::create_tag(),
                                        block_header.to_json()
                                }
                        )
                    ).get()
            );
            blocks.push_back(Block::from_json(block_response.content()));
        }
        return blocks;
    };

    auto block_chain_from_specific_node = [&]() -> ErrorOr<void> {

        auto peers = TRY(manager.get_peers({PeerType::Full}));
        if (peers.empty())
            return tl::make_unexpected(KrapiErr{"There are no peers connected."});

        for (int counter = 1; const auto &[id, type, state]: peers) {

            fmt::print(
                    "{}. Peer: {}, Type: {}, State: {}\n",
                    counter++,
                    id,
                    to_string(type),
                    to_string(state)
            );
        }
        int choice;
        fmt::print("Enter choice (1 - {}): ", peers.size());
        std::cin >> choice;
        if (choice < 1 || choice > peers.size())
            return tl::make_unexpected(KrapiErr{"Wrong Choice"});

        auto selected_peer = std::get<0>(peers[choice - 1]);
        fmt::print("Getting chain from {}\n", selected_peer);
        auto blocks = TRY(get_blocks(selected_peer, genesis_header));

        if (auto val = print_chain(blocks); !val.has_value())
            return tl::make_unexpected(val.error());

        return {};
    };
    auto block_chain_from_all_nodes = [&]() -> ErrorOr<void> {
        auto peers = TRY(manager.get_peers({PeerType::Full}));
        if (peers.empty())
            return tl::make_unexpected(KrapiErr{"There are no peers connected."});

        for (const auto &[id, type, state]: peers) {
            fmt::print("{:*^50}\n", fmt::format("Peer #{}", id));
            auto blocks = TRY(get_blocks(id, genesis_header));
            print_chain(blocks);
            fmt::print("{:*^50}\n", fmt::format("Peer #{}", id));
        }
        return {};
    };

    auto tip_from_specific_node = [&]() -> ErrorOr<void> {
        auto peers = TRY(manager.get_peers({PeerType::Full}));
        if (peers.empty())
            return tl::make_unexpected(KrapiErr{"There are no peers connected."});

        for (int counter = 1; const auto &[id, type, state]: peers) {

            fmt::print(
                    "{}. Peer: {}, Type: {}, State: {}\n",
                    counter++,
                    id,
                    to_string(type),
                    to_string(state)
            );
        }
        int choice;
        fmt::print("Enter choice (1 - {}): ", peers.size());
        std::cin >> choice;
        if (choice < 1 || choice > peers.size())
            return tl::make_unexpected(KrapiErr{"Wrong Choice"});

        auto selected_peer = std::get<0>(peers[choice - 1]);
        fmt::print("Requesting tip from peer {}\n", selected_peer);

        auto response = TRY(
                TRY(
                        manager.send(
                            selected_peer,
                            PeerMessage{
                                    PeerMessageType::GetLastBlockRequest,
                                    manager.id(),
                                    PeerMessage::create_tag()
                            }
                    )
                ).get()
        );
        fmt::print("{:*^50}\n", fmt::format("Peer #{}", selected_peer));
        fmt::print("{}\n", response.content().dump(4));
        fmt::print("{:*^50}\n", fmt::format("Peer #{}", selected_peer));
        return {};
    };

    auto tip_from_all_nodes = [&]() -> ErrorOr<void> {
        auto responses = manager.broadcast(
                PeerMessage{
                        PeerMessageType::GetLastBlockRequest,
                        manager.id()
                }
        ).get();
        for (const auto &resp: responses) {

            if (!resp.has_value()) {
                continue;
            }
            auto block = Block::from_json(resp.value().content());
            fmt::print("{:*^50}\n", fmt::format("Peer #{}", resp.value().peer_id()));
            fmt::print("{}\n", block.to_json().dump(4));
            fmt::print("{:*^50}\n", fmt::format("Peer #{}", resp.value().peer_id()));
        }
        return {};
    };


    while (true) {
        fmt::print("{}", fmt::join(options, "\n"));
        fmt::print("\nEnter your choice (1-{}): ", options.size());
        int choice;
        std::cin >> choice;
        switch (choice) {
            case 1:
                if (auto val = block_chain_from_specific_node(); !val.has_value()) {

                    fmt::print("ERROR: {}\n", val.error().err_str);
                }
                break;
            case 2:
                if (auto val = block_chain_from_all_nodes(); !val.has_value()) {

                    fmt::print("ERROR: {}\n", val.error().err_str);
                }
                break;
            case 3:
                if (auto val = tip_from_specific_node();!val.has_value()) {

                    fmt::print("ERROR: {}\n", val.error().err_str);
                }
                break;
            case 4:
                if (auto val = tip_from_all_nodes(); !val.has_value()) {
                    fmt::print("ERROR: {}\n", val.error().err_str);
                }
                break;
            default:
                break;
        }
    }
}