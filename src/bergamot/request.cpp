#include "common/logging.h"
#include "request.h"
#include "definitions.h"
#include "translation_result.h"
#include "sys/time.h"
#include <iostream>

namespace marian {
namespace bergamot {

Request::Request(unsigned int Id, 
                 std::vector<Ptr<Vocab const>> vocabs,
                 string_view reference,
                 Ptr<Segments> segments,
                 Ptr<SourceAlignments> sourceAlignments,
                 Ptr<std::promise<TranslationResult>> translationResultPromise)
    : Id(Id),
      vocabs_(vocabs), reference_(reference), 
      segments(segments),
      sourceAlignments(sourceAlignments),
      response_(translationResultPromise), 
      counter_(segments->size()) {

      for(int i=0; i < segments->size(); i++){
          histories_.push_back(nullptr);
      }

}

int Request::size(){ 
  return segments->size(); 
};

void Request::set_translation(int index, Ptr<History> history) {
  /* This can be accessed by multiple batch_translators at once. */
  // std::lock_guard<std::mutex> request_lock(update_mutex_);
  histories_[index] = history;
  if(--counter_ == 0){
    TranslationResult translation_result;
    for(int i=0; i < segments->size(); i++){
      translation_result.sources.push_back(
          vocabs_.front()->decode(segments->at(i))
      );

      history = histories_[i];
      NBestList onebest = history->nBest(1);
      Result result = onebest[0];  // Expecting only one result;
      Words words = std::get<0>(result);
      translation_result.translations.push_back(
          vocabs_.back()->decode(words)
     );
    
    }
    LOG(info, "Last translation in. Closing request;");
    response_->set_value(translation_result);
  }
}

bool operator<(const Request &a, const Request &b) {
  // TODO(jerin): Probably enhance
  return a.Id < b.Id;
}

RequestSentence::RequestSentence(int index, 
                                 Ptr<Request> request)
    : index(index), request(request) {}

int RequestSentence::num_tokens(){
  return (request->segments->at(index).size());
}

Segment RequestSentence::segment() const {
  return request->segments->at(index);
}

bool operator<(const RequestSentence& a, const RequestSentence& b) {
  if(a.request == b.request){
    return a.index < b.index;
  }
  return a < b;
}

bool operator==(const RequestSentence& a, const RequestSentence& b) {
  return (a.index == b.index) and (a.request == b.request);
};
}  // namespace bergamot
}  // namespace marian
