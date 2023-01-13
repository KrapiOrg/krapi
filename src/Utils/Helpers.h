#pragma once

#include <chrono>
#include <cstdint>
namespace krapi {

  inline uint64_t get_krapi_timestamp() {
    return duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch()
    )
      .count();
  }

}// namespace krapi