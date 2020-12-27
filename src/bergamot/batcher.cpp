#include "batcher.h"
#include "common/logging.h"
#include <cassert>

namespace marian {
namespace bergamot {

Batcher::Batcher(Ptr<Options> options) {
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
  // std::cout << "bucket_id " << bucket_id 
  //     << " max_input_sentence_tokens_ " << max_input_sentence_tokens_ << std::endl;
  assert(bucket_id <= max_input_sentence_tokens_);
  bucket[bucket_id].insert(priority);
}


void Batcher::cleave_batch(Ptr<Segments> segments, 
                           Ptr<std::vector<MultiFactorPriority>> sentences) {
  /* Temporary stub, needs improvement this section */
  int segments_added = 0;
  int current_input_tokens = 0;
  int padded_batch_size = 0;
  int prev_padded_batch_size;
  for (int i = 0; i < bucket.size(); i++) {
    auto p = bucket[i].begin();
    while ( p != bucket[i].end() ){
      padded_batch_size = (segments_added+1)*i;
      if (padded_batch_size < max_input_tokens_){
        auto q = p;
        current_input_tokens += i;
        segments->push_back(q->segment());
        sentences->push_back(*q);
        ++p;
        ++segments_added;
        bucket[i].erase(q);
        prev_padded_batch_size = padded_batch_size;
      }
      else{
        LOG(info, "New batch generated; {} Segments added;", segments_added);
        LOG(info, 
            "padded_batch_size ({}) current_input_tokens({})", 
            prev_padded_batch_size, current_input_tokens);
        return;
      }
    }
  }
}

}  // namespace bergamot
}  // namespace marian
