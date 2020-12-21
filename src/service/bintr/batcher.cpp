#include "batcher.h"


namespace marian {
  namespace bergamot {
Batcher::Batcher(Ptr<Options> options) : current_input_tokens_(0) {
  /* Reads options for batch configuration */
  max_input_tokens_ = options->get<int>("max-input-tokens");
  maxi_batch_size_ = options->get<int>("maxi-batch-size");
  bucket.reserve(max_input_tokens_ + 1);
}

void Batcher::addSentenceWithPriority(MultiFactorPriority &priority){
  bucket[priority.num_tokens()].push(priority);
}


std::vector<data::SentenceTuple> Batcher::cleave_batch() {
  /* Construct one-to-one mapping here */
  /* This is a dynamic programming approach to optimize for a
    * weighted padding + priority objective tentatively */
}

}
}
