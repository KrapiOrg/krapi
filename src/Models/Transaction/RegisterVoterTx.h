//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_REGISTERVOTERTX_H
#define KRAPI_MODELS_REGISTERVOTERTX_H

#include "TransactionBase.h"

namespace krapi {

    class RegisterVoterTx : public TransactionBase {
    public:
        explicit RegisterVoterTx() : TransactionBase("register_voter") {

        }

        [[nodiscard]]
        nlohmann::json to_json() const override {

            return TransactionBase::to_json();
        }

    };

} // krapi

#endif //KRAPI_MODELS_REGISTERVOTERTX_H
