#pragma once
#include "common/options.h"
#include "data/corpus_base.h"

#include <queue>
#include <vector>
#include <set>


namespace marian {
  namespace bergamot {
    struct MultiFactorPriority;

    class Batcher {
      unsigned int max_input_tokens_;
      unsigned int max_input_sentence_tokens_;
      unsigned int current_input_tokens_;
      std::vector<std::set<MultiFactorPriority>> bucket;

      public:
      Batcher(Ptr<Options> options);
      void addSentenceWithPriority(MultiFactorPriority &);
      std::vector<data::SentenceTuple> cleave_batch();
    };
  }
}

#include "multifactor_priority.h"
