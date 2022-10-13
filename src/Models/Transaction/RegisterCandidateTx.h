//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_REGISTERCANDIDATETX_H
#define KRAPI_MODELS_REGISTERCANDIDATETX_H

#include "TransactionBase.h"

namespace krapi {
    struct RegisterCandidateTx : public TransactionBase {
        explicit RegisterCandidateTx() : TransactionBase("register_candidate") {}

        [[nodiscard]]
        nlohmann::json to_json() const override {

            return TransactionBase::to_json();
        }
    };
}
#endif //KRAPI_MODELS_REGISTERCANDIDATETX_H
