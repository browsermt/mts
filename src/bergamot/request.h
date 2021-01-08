#ifndef __BERGAMOT_REQUEST_H
#define __BERGAMOT_REQUEST_H

#include "data/types.h"
#include "sys/time.h"
#include "translation_result.h"
#include "definitions.h"
#include <future>
#include "translator/beam_search.h"
#include <vector>
#include <mutex>



namespace marian {
namespace bergamot {

struct Request {
  string_view reference_;
  Ptr<Segments> segments;
  Ptr<SourceAlignments> sourceAlignments;
  Ptr<std::promise<TranslationResult>> response_;
  timeval created;
  std::vector<Ptr<History>> histories_;
  std::atomic<int> counter_;


  // @TODO(jerin): This is a bit weird, need to do better.
  std::vector<Ptr<Vocab const>> vocabs_; 

  Request(std::vector<Ptr<Vocab const>>,
          string_view, 
          Ptr<Segments>, 
          Ptr<SourceAlignments>,
          Ptr<std::promise<TranslationResult>>);
  void set_translation(int index, Ptr<History>);
  int size();
};

struct RequestSentence {
  /* A sentence tied to a request. */
  int index;
  Ptr<Request> request;
  RequestSentence(int, Ptr<Request>);
  int num_tokens();
  Segment segment() const;

};

bool operator<(const RequestSentence& a, const RequestSentence& b);
bool operator==(const RequestSentence& a, const RequestSentence& b);

typedef std::vector<RequestSentence> RequestSentences;

struct PCItem {
  Ptr<Segments> segments;
  Ptr<RequestSentences> sentences;
  PCItem(): segments(NULL), sentences(NULL) {}
  PCItem(Ptr<Segments> segments, Ptr<RequestSentences> sentences):
      segments(segments), sentences(sentences){}

  void operator=(const PCItem &b){
    segments = b.segments;
    sentences = b.sentences;
  }

  bool isPoison(){
      return (segments == NULL);
  }

};



}  // namespace bergamot
}  // namespace marian

#endif // __BERGAMOT_REQUEST_H
