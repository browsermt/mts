#pragma once
#include "common/options.h"
#include "data/corpus_base.h"

#include <queue>
#include <vector>


namespace marian {
  namespace bergamot {
    struct MultiFactorPriority;

    class Batcher {
      unsigned int max_input_tokens_;
      unsigned int maxi_batch_size_;
      unsigned int current_input_tokens_;
      std::vector<std::priority_queue<MultiFactorPriority>> bucket;

      public:
      Batcher(Ptr<Options> options);
      void addSentenceWithPriority(MultiFactorPriority &);
      std::vector<data::SentenceTuple> cleave_batch();
    };
  }
}

#include "multifactor_priority.h"
