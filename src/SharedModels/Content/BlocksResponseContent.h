//
// Created by mythi on 19/11/22.
//

#pragma once

#include "Block.h"
#include <list>
#include <span>

namespace krapi {
  struct BlocksResponseContent {
    std::list<Block> m_blocks;

   public:
    explicit BlocksResponseContent(std::list<Block> blocks)
        : m_blocks(std::move(blocks)) {}

    [[nodiscard]] std::list<Block> blocks() const { return m_blocks; }

    static BlocksResponseContent from_json(nlohmann::json json) {

      auto blocks = std::list<Block>{};

      for (const auto &json_block: json["blocks"]) {
        blocks.push_back(Block::from_json(json_block));
      }

      return BlocksResponseContent{std::move(blocks)};
    }

    [[nodiscard]] nlohmann::json to_json() const {

      auto json = nlohmann::json::array();
      for (const auto &block: m_blocks) { json.push_back(block.to_json()); }

      return {{"blocks", json}};
    }
  };
}// namespace krapi