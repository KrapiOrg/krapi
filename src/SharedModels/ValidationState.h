#pragma once

#include "nlohmann/json.hpp"

namespace krapi {
  enum class ValidationState {
    Success,
    TransactionAlreadySpent,
    WrongTimestamp,
    IncompeleteProofOfWork,
    WrongPreviousHash
  };
  NLOHMANN_JSON_SERIALIZE_ENUM(
    ValidationState,
    {{ValidationState::Success, "success"},
     {ValidationState::TransactionAlreadySpent, "transaction_already_spent"},
     {ValidationState::WrongTimestamp, "wrong_timestamp"},
     {ValidationState::IncompeleteProofOfWork, "incompelete_proof_of_work"},
     {ValidationState::WrongPreviousHash, "wrong_previous_hash"}}
  )

  inline std::string to_string(ValidationState reason) {
    switch (reason) {
      case ValidationState::Success:
        return "success";
      case ValidationState::TransactionAlreadySpent:
        return "transaction_already_spent";
      case ValidationState::WrongTimestamp:
        return "wrong_time_stamp";
      case ValidationState::IncompeleteProofOfWork:
        return "incompelete_proof_of_work";
      case ValidationState::WrongPreviousHash:
        return "wrong_previous_hash";
    }
  }


}// namespace krapi