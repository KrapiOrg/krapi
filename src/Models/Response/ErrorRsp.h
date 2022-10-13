//
// Created by mythi on 14/10/22.
//

#ifndef RSPNS_MODELS_ERRORRSP_H
#define RSPNS_MODELS_ERRORRSP_H

#include "ResponseBase.h"

namespace krapi {
    struct ErrorRsp : public ResponseBase {
    public:
        explicit ErrorRsp() : ResponseBase("error_response") {}

        [[nodiscard]]
        nlohmann::json to_json() const override {

            return ResponseBase::to_json();
        }
    };
}
#endif //RSPNS_MODELS_ERRORRSP_H
