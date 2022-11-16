//
// Created by mythi on 12/11/22.
//

#pragma once

#include <functional>
#include "Block.h"
#include "eventpp/eventdispatcher.h"
#include "merklecpp.h"
#include "sha.h"

namespace krapi {

    class Miner {
    public:
        enum class Event {
            BlockMined,
        };
    private:
        using OnBlockMinedCallback = void(Block);
        using BlockEventsDispatcher = eventpp::EventDispatcher<Event, OnBlockMinedCallback>;
        BlockEventsDispatcher m_dispatcher;
        static inline CryptoPP::SHA256 sha_256;
        static inline void krapi_hash_function(
                const merkle::HashT<32> &,
                const merkle::HashT<32> &,
                merkle::HashT<32> &
        );

        std::mutex miner_mutex;
        Block m_last_mined;
    public:

        explicit Miner(Block last_mined);

        void mine(std::unordered_set<Transaction>);

        void append_listener(Event, const std::function<OnBlockMinedCallback> &callback);
    };

} // krapi
