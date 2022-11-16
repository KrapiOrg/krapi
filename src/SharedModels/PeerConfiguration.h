//
// Created by mythi on 16/11/22.
//

#ifndef SHARED_MODELS_PEERCONFIGURATION_H
#define SHARED_MODELS_PEERCONFIGURATION_H

#include "nlohmann/json.hpp"

namespace krapi {
    enum class PeerType {
        Full,
        Light
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(PeerType, {
        { PeerType::Full, "peer_type_full" },
        { PeerType::Full, "peer_type_light" },
    })

    class PeerConfiguration {
        int m_id;
        PeerType m_type;
    public:
        explicit PeerConfiguration(int id = 0, PeerType type = PeerType::Full) : m_id(id), m_type(type) {

        }

        [[nodiscard]]
        int id() const {

            return m_id;
        }

        [[nodiscard]]
        PeerType type() const {

            return m_type;
        }

        [[nodiscard]]
        nlohmann::json to_json() const {

            return {
                    {"id",   m_id},
                    {"type", m_type}
            };
        }

        static PeerConfiguration from_json(nlohmann::json json) {

            return PeerConfiguration{
                    json["id"].get<int>(),
                    json["type"].get<PeerType>(),
            };
        }

        bool operator==(const PeerConfiguration &other) const = default;

    };


}
namespace std {
    template<>
    struct hash<krapi::PeerConfiguration> {
        size_t operator()(const krapi::PeerConfiguration &config) const {

            return hash<int>{}(config.id()) * 31 + hash<krapi::PeerType>{}(config.type());
        }
    };
}
#endif //SHARED_MODELS_PEERCONFIGURATION_H
