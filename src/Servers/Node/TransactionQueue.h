//
// Created by mythi on 26/10/22.
//

#ifndef NODE_TRANSACTIONQUEUE_H
#define NODE_TRANSACTIONQUEUE_H

#include <memory>
#include "eventpp/eventdispatcher.h"
#include "Transaction.h"

namespace krapi {
    using TransactionQueue = eventpp::EventDispatcher<int, void(Transaction)>;
    using TransactionQueuePtr = std::shared_ptr<TransactionQueue>;

    inline TransactionQueuePtr create_tx_queue() {

        return std::make_shared<TransactionQueue>();
    }

}

#endif //NODE_TRANSACTIONQUEUE_H
