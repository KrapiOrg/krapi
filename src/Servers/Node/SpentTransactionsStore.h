#pragma once

#include "DBInterface.h"
#include "EventQueue.h"
#include "InternalMessage.h"
#include "PeerMessage.h"
#include "Transaction.h"
#include "eventpp/utilities/scopedremover.h"
#include "spdlog/spdlog.h"
#include <memory>
namespace krapi {
  class SpentTransactionsStore : public DBInternface<Transaction> {

   public:
    static std::shared_ptr<SpentTransactionsStore> create(
      std::string path
    ) {
      return std::shared_ptr<SpentTransactionsStore>(
        new SpentTransactionsStore(std::move(path))
      );
    }


   private:
    SpentTransactionsStore(std::string path) {
      if (!initialize(path)) {
        spdlog::error("Failed to open spent transactions database");
        exit(1);
      }
    }
  };

  using SpentTransactionsStorePtr = std::shared_ptr<SpentTransactionsStore>;
}// namespace krapi