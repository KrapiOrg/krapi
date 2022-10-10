#include "spdlog/spdlog.h"
#include "Utils/ErrorOr.h"



namespace krapi {
    ErrorOr<int> krapi_main() {

        return KrapiErr{""sv,KrapiCode::CRITICAL};
    }
}

int main() {

    auto ret_val = krapi::krapi_main();
    if (ret_val.is_error()) {
        const auto &err = ret_val.error();
        switch (err.code) {

            case krapi::KrapiCode::WARN:
                spdlog::warn(err.err_str);
                break;
            case krapi::KrapiCode::INFO:
                spdlog::info(err.err_str);
                break;
            case krapi::KrapiCode::CRITICAL:
                spdlog::critical(err.err_str);
                break;
            case krapi::KrapiCode::ERROR:
                spdlog::error(err.err_str);
                break;
        }
        return static_cast<int>(err.code);
    }

    return 0;
}

