#include "batcher.h"
#include "common/logging.h"
#include "sanelogging.h"
#include <cassert>

namespace marian {
namespace bergamot {

Batcher::Batcher(Ptr<Options> options) {
  /* Reads options for batch configuration */
  max_input_tokens_ = options->get<int>("max-input-tokens");
  max_input_sentence_tokens_ = options->get<int>("max-input-sentence-tokens");
  bucket.reserve(max_input_sentence_tokens_ + 1);
  for (int i = 0; i <= max_input_sentence_tokens_; i++) {
    bucket.push_back(std::set<RequestSentence>());
  }
}

void Batcher::addSentenceWithPriority(RequestSentence &sentence) {
  int bucket_id = sentence.numTokens();
  assert(bucket_id <= max_input_sentence_tokens_);
  bucket[bucket_id].insert(sentence);
}

void Batcher::cleave_batch(Ptr<RequestSentences> sentences) {
  /* Temporary stub, needs improvement this section */
  int segments_added = 0;
  int current_input_tokens = 0;
  int padded_batch_size = 0;
  int prev_padded_batch_size;
  for (int i = 0; i < bucket.size(); i++) {
    auto p = bucket[i].begin();
    while (p != bucket[i].end()) {
      padded_batch_size = (segments_added + 1) * i;
      if (padded_batch_size < max_input_tokens_) {
        auto q = p;
        ++p;
        current_input_tokens += i;
        sentences->push_back(*q);
        ++segments_added;
        bucket[i].erase(q);
        prev_padded_batch_size = padded_batch_size;
      } else {
        PLOG("main", info, "New batch generated; {} Segments added;",
             segments_added);
        PLOG("main", info, "padded_batch_size ({}) current_input_tokens({})",
             prev_padded_batch_size, current_input_tokens);
        return;
      }
    }
  }
}

} // namespace bergamot
} // namespace marian
