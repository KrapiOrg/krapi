//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_VOTETX_H
#define KRAPI_MODELS_VOTETX_H

#include "TransactionBase.h"

namespace krapi {

    class VoteTx : public TransactionBase {
    public:
        explicit VoteTx() : TransactionBase("vote") {}

        [[nodiscard]]
        nlohmann::json to_json() const override {

            return TransactionBase::to_json();
        }
    };

} // krapi

#endif //KRAPI_MODELS_VOTETX_H
