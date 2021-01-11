#ifndef __BERGAMOT_BATCHER_H
#define __BERGAMOT_BATCHER_H

#include "common/options.h"
#include "data/corpus_base.h"
#include "definitions.h"
#include "request.h"

#include <set>
#include <vector>

namespace marian {
namespace bergamot {
struct RequestSentence;

class Batcher {
  unsigned int max_input_tokens_;
  unsigned int max_input_sentence_tokens_;
  std::vector<std::set<RequestSentence>> bucket;

public:
  explicit Batcher(Ptr<Options> options);
  void addSentenceWithPriority(RequestSentence &);
  void cleave_batch(Ptr<RequestSentences>);
};

} // namespace bergamot
} // namespace marian

#endif // __BERGAMOT_BATCHER_H
