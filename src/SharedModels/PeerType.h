#pragma once

#include "nlohmann/json.hpp"

namespace krapi {
  enum class PeerType {
    Light,
    Full,
    Observer,
    Control,
    Unknown
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(
    PeerType,
    {{PeerType::Full, "full"},
     {PeerType::Light, "light"},
     {PeerType::Observer, "observer"},
     {PeerType::Control, "control"},
     {PeerType::Unknown, "unknown"}}
  )

  inline std::string to_string(PeerType type) {

    if (type == PeerType::Full) return "full";
    if (type == PeerType::Light) return "light";
    if (type == PeerType::Control) return "control";
    if (type == PeerType::Observer) return "observer";

    return "unknown";
  }

}// namespace krapi
