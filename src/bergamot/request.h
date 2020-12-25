#ifndef __BERGAMOT_REQUEST_H
#define __BERGAMOT_REQUEST_H
#include "data/types.h"
#include "sys/time.h"
#include "translation_result.h"
#include "definitions.h"
#include <future>
#include "translator/beam_search.h"
#include <unordered_map>
#include <mutex>



namespace marian {
namespace bergamot {

struct Request {
  string_view reference_;
  Ptr<std::promise<TranslationResult>> response_;
  Ptr<Segments> segments;
  std::unordered_map<int, std::string> translations;
  Ptr<Alignments> alignments;
  timeval created;
  bool cancelled_;
  std::mutex update_mutex_;


  Request(string_view, Ptr<Segments>, Ptr<Alignments>,
          Ptr<std::promise<TranslationResult>> );
  void join();
  void cancel();
  int size();
  void set_translation(int index, std::string);
  bool cancelled();
};

}  // namespace bergamot
}  // namespace marian

#endif // __BERGAMOT_REQUEST_H
