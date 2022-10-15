//
// Created by mythi on 14/10/22.
//

#ifndef RSPNS_MODELS_ERRORRSP_H
#define RSPNS_MODELS_ERRORRSP_H

#include "ResponseBase.h"

namespace krapi {
    struct ErrorRsp : public ResponseBase {
        explicit ErrorRsp() : ResponseBase("error_rsp") {}
    };
}
#endif //RSPNS_MODELS_ERRORRSP_H
