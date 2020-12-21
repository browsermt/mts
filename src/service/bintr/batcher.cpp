#include "batcher.h"
#include <cassert>

namespace marian {
  namespace bergamot {

    Batcher::Batcher(Ptr<Options> options) : current_input_tokens_(0) {
      /* Reads options for batch configuration */
      max_input_tokens_ = options->get<int>("max-input-tokens");
      max_input_sentence_tokens_ = options->get<int>("max-input-sentence-tokens");
      bucket.reserve(max_input_sentence_tokens_ + 1);
      for(int i=0; i<=max_input_sentence_tokens_; i++){
        bucket.push_back(std::set<MultiFactorPriority>());
      }
    }

    void Batcher::addSentenceWithPriority(MultiFactorPriority &priority){
      int bucket_id = priority.num_tokens();
      assert(bucket_id <= max_input_sentence_tokens_);
      bucket[bucket_id].insert(priority);
    }


    std::vector<data::SentenceTuple> Batcher::cleave_batch() {
      /* Construct one-to-one mapping here */
      /* This is a dynamic programming approach to optimize for a
       * weighted padding + priority objective tentatively */
    }

  }
}
