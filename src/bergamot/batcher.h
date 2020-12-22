#ifndef __BERGAMOT_BATCHER_H
#define __BERGAMOT_BATCHER_H

#include "common/options.h"
#include "data/corpus_base.h"
#include "definitions.h"

#include <queue>
#include <vector>
#include <set>


namespace marian {
namespace bergamot {
struct MultiFactorPriority;

class Batcher {
  unsigned int max_input_tokens_;
  unsigned int max_input_sentence_tokens_;
  std::vector<std::set<MultiFactorPriority>> bucket;

 public:
  explicit Batcher(Ptr<Options> options);
  void addSentenceWithPriority(MultiFactorPriority &);
  void cleave_batch(Ptr<Segments>, Ptr<std::vector<MultiFactorPriority>>);
};

}  // namespace bergamot
}  // namespace marian

#include "multifactor_priority.h"

#endif // __BERGAMOT_BATCHER_H
