//
// Created by mythi on 12/11/22.
//

#pragma once

#include <future>
#include <functional>
#include "Block.h"
#include "eventpp/eventdispatcher.h"
#include "merklecpp.h"
#include "sha.h"


namespace krapi {

    class Miner {
    public:
        enum class Event {
            BlockMined
        };
        enum class BatchEvent {
            BatchSubmitted,
            BatchCancelled
        };
    private:
        using OnBlockMinedCallback = void(Block);
        using OnBatchEventCallback = void(std::unordered_set<Transaction>);
        using BlockEventsDispatcher = eventpp::EventDispatcher<Event, OnBlockMinedCallback>;
        using BatchEventDispatcher = eventpp::EventDispatcher<BatchEvent, OnBatchEventCallback>;

        BlockEventsDispatcher m_dispatcher;
        BatchEventDispatcher m_batch_dispatcher;

        static inline CryptoPP::SHA256 sha_256;

        static inline void krapi_hash_function(
                const merkle::HashT<32> &,
                const merkle::HashT<32> &,
                merkle::HashT<32> &
        );

        void async_mine(std::unordered_set<Transaction>);

        std::mutex m_mutex;
        std::vector<std::future<void>> m_futures;
        std::vector<std::unordered_set<Transaction>> m_batches;

        std::string m_latest_hash;
        std::atomic<bool> m_cancelled;

    public:
        explicit Miner();

        void mine(std::unordered_set<Transaction>);

        void cancel_all();

        void set_latest_hash(std::string);

        void append_listener(Event, const std::function<OnBlockMinedCallback> &callback);

        void append_listener(BatchEvent, const std::function<OnBatchEventCallback> &callback);
    };

} // krapi
