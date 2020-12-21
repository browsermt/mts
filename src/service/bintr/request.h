#pragma once
#include "data/types.h"
#include "sys/time.h"
#include "translation_result.h"
#include "definitions.h"
#include <future>
#include "translator/beam_search.h"


namespace marian {
namespace bergamot {


struct Request {
  /* Wraps around a request, to execute barrier/synchronization */
  std::promise<TranslationResult> *response_;
  Ptr<std::vector<Segment>> segments;
  Ptr<Alignments> alignments;
  timeval created;
  Request(Ptr<std::vector<Segment>>, Ptr<Alignments>, std::promise<TranslationResult> &);
  void join();
  void cancel();
  void set_translation(int index, Ptr<History> history);
};
}  // namespace bergamot
}  // namespace marian
