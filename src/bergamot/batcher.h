#ifndef SRC_BERGAMOT_BATCHER_H_
#define SRC_BERGAMOT_BATCHER_H_

#include "common/options.h"
#include "data/corpus_base.h"
#include "definitions.h"
#include "request.h"

#include <set>
#include <vector>

namespace marian {
namespace bergamot {
class Batcher {
  unsigned int max_input_tokens_;
  unsigned int max_input_sentence_tokens_;
  std::vector<std::set<RequestSentence>> bucket;

public:
  explicit Batcher(Ptr<Options> options);
  void addSentenceWithPriority(RequestSentence &);
  void cleave_batch(RequestSentences &);
};

} // namespace bergamot
} // namespace marian

#endif // SRC_BERGAMOT_BATCHER_H_
