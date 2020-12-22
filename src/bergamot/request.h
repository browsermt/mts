#pragma once
#include "data/types.h"
#include "sys/time.h"
#include "translation_result.h"
#include "definitions.h"
#include <future>
#include "translator/beam_search.h"
#include <unordered_map>



namespace marian {
namespace bergamot {

struct Request {
  string_view reference_;
  std::promise<TranslationResult> *response_;
  Ptr<std::vector<Segment>> segments;
  std::unordered_map<int, std::string> translations;
  Ptr<Alignments> alignments;
  timeval created;
  bool cancelled_;


  Request(string_view, Ptr<Segments>, Ptr<Alignments>,
          std::promise<TranslationResult> &);
  void join();
  void cancel();
  int size();
  void set_translation(int index, std::string);
  bool cancelled();
};

}  // namespace bergamot
}  // namespace marian
