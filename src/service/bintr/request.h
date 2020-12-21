#pragma once
#include "data/types.h"
#include "sys/time.h"
#include "translation_result.h"
#include "definitions.h"
#include <future>


namespace marian {
namespace bergamot {


struct Request {
  /* Wraps around a request, to execute barrier/synchronization */
  std::promise<TranslationResult> *response_;
  std::vector<Segment> *segments;
  timeval *created;
  Request(std::vector<Segment> &, std::promise<TranslationResult> &);
  void cancel();
  void join();
  void queue_segments();

};
}  // namespace bergamot
}  // namespace marian
