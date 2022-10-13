//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_TRANSACTION_H
#define KRAPI_MODELS_TRANSACTION_H

#include <variant>
#include "RegisterCandidateTx.h"
#include "RegisterVoterTx.h"
#include "VoteTx.h"

namespace krapi {
    using Transaction = std::variant<RegisterCandidateTx, RegisterVoterTx, VoteTx>;
}

#endif //KRAPI_MODELS_TRANSACTION_H
