#pragma once

#include "nlohmann/json.hpp"

namespace krapi {
  enum class PeerState {
    InitialBlockDownload,
    WaitingForPeers,
    Closed,
    Open,
    Error,
    Unknown
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(
    PeerState,
    {
      {PeerState::InitialBlockDownload, "initial_block_download"},
      {PeerState::WaitingForPeers, "waiting_for_peers"},
      {PeerState::Closed, "closed"},
      {PeerState::Open, "open"},
      {PeerState::Error, "error"},
      {PeerState::Unknown, "unknown"},
    }
  )

  inline std::string to_string(PeerState state) {

    switch (state) {

      case PeerState::InitialBlockDownload:
        return "initial_block_download";
      case PeerState::WaitingForPeers:
        return "waiting_for_peers";
      case PeerState::Closed:
        return "closed";
      case PeerState::Open:
        return "open";
      case PeerState::Error:
        return "error";
      case PeerState::Unknown:
        return "unknown";
    }
    return "error";
  }
}// namespace krapi