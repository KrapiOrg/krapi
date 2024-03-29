#pragma once

#include "BlockHeader.h"
#include <vector>

namespace krapi {
  struct BlockHeadersResponseContent {
    std::vector<BlockHeader> m_headers;

   public:
    explicit BlockHeadersResponseContent(std::vector<BlockHeader> headers)
        : m_headers(std::move(headers)) {
    }

    [[nodiscard]] std::vector<BlockHeader> headers() const {
      return m_headers;
    }

    static BlockHeadersResponseContent from_json(nlohmann::json json) {

      auto headers = std::vector<BlockHeader>{};

      for (const auto &json_block: json["headers"]) {
        headers.push_back(BlockHeader::from_json(json_block));
      }

      return BlockHeadersResponseContent{std::move(headers)};
    }

    [[nodiscard]] nlohmann::json to_json() const {

      auto json = nlohmann::json::array();
      for (const auto &block: m_headers) {
        json.push_back(block.to_json());
      }

      return {{"headers", json}};
    }
  };
}// namespace krapi