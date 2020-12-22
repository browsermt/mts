#include "request.h"
#include "definitions.h"
#include "translation_result.h"
#include "sys/time.h"
#include <iostream>

namespace marian {
namespace bergamot {
Request::Request(string_view reference,
                 Ptr<std::vector<Segment>> segments,
                 Ptr<Alignments> alignments,
                 std::promise<TranslationResult> &translation_result_promise):
    reference_(reference), segments(segments),
    alignments(alignments),
    cancelled_(false) {
      /* Construction should mean item is queued for translation  */
      struct timezone *tz = NULL;
      gettimeofday(&created, tz);
      response_ = &translation_result_promise;
    }

void Request::cancel() {
  cancelled_ = true;
}

bool Request::cancelled() {
  return cancelled_;
}

int Request::size(){ 
  return segments->size(); 
};

void Request::set_translation(int index, std::string translation) {
  translations[index] = translation;
}

void Request::join() {
  TranslationResult translation_result;
  for(int i=0; i < segments->size(); i++){
    translation_result.sources.push_back("");
    translation_result.translations.push_back(translations[i]);
  
  }
  response_->set_value(translation_result);
}

bool operator<(const Request &a, const Request &b) {
  // TODO(jerin): Probably enhance
  return a.created.tv_sec < b.created.tv_sec;
}

}  // namespace bergamot
}  // namespace marian
