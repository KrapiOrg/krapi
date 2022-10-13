//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_JSONTOTXCONVERTER_H
#define KRAPI_MODELS_JSONTOTXCONVERTER_H

#include "Transaction.h"

namespace krapi {
    struct JsonToTxConverter {
        Transaction operator()(const nlohmann::json &json) {

            if (json["type"] == "register_candidate") {
                return RegisterCandidateTx{};
            } else if (json["type"] == "register_voter") {
                return RegisterVoterTx{};
            }
            return VoteTx{};
        }


    };
}
#endif //KRAPI_MODELS_JSONTOTXCONVERTER_H
