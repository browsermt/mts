//
// Defines:
//
// Request: holds the input blob of a text, Segments (vector<Words>) which are
// to go to the batching mechanism and alignments between the processed
// segments and the input blob (sourceAlignments). In addition, Request takes
// care of the barrier which fires when all the Segments in a request are done
// translating by the workers (BatchTranslator). Request is to be extended with
// notions of Priority (sequence, user-given).
//
// RequestSentence: is a mapping of (index, Request*). This provides the
// batching mechanism access to the segment within the request. The backref to
// Request allows event triggering the barrier upon completion of the last
// sentence by a worker.
//
// PCItem: is a vector of RequestSentences and the corresponding Segments,
// which is what the ProducerConsumer queue holds. Can probably get rid of
// Segment here and use RequestSentence directly to construct batches.
// Separation is worker(BatchTranslator) need not be aware of the notion of
// Request, but only a Batch of segments.

#ifndef SRC_BERGAMOT_REQUEST_H_
#define SRC_BERGAMOT_REQUEST_H_

#include "definitions.h"
#include "translation_result.h"

#include "data/types.h"
#include "translator/beam_search.h"

#include <future>
#include <vector>

namespace marian {
namespace bergamot {

class Request {
private:
  unsigned int Id_;
  string_view reference_;
  Ptr<Segments> segments_;
  Ptr<SourceAlignments> sourceAlignments_;
  Ptr<std::promise<TranslationResult>> response_;
  std::vector<Ptr<History>> histories_;
  std::atomic<int> counter_;

  // @TODO(jerin): This is a bit weird, need to do better.
  std::vector<Ptr<Vocab const>> vocabs_;

public:
  Request(unsigned int, std::vector<Ptr<Vocab const>>, string_view,
          Ptr<Segments>, Ptr<SourceAlignments>,
          Ptr<std::promise<TranslationResult>>);

  void processHistory(int index, Ptr<History>);
  void completeRequest();

  // Obtain the count of tokens in a segment. Used to insert sentence from
  // multiple requests into the corresponding size bucket.
  int segmentTokens(int) const;

  // Obtain number of segments in a request.
  int numSegments() const;

  // Obtains a segment to create a batch of segments among several requests.
  Segment getSegment(int) const;

  bool operator<(const Request &) const;
};

class RequestSentence {
private:
  int index_;
  Ptr<Request> request_;

public:
  RequestSentence(int, Ptr<Request>);
  int numTokens();
  Segment getUnderlyingSegment() const;
  void completeSentence(Ptr<History>);
  friend bool operator<(const RequestSentence &, const RequestSentence &);
};

typedef std::vector<RequestSentence> RequestSentences;

struct PCItem {
  int batchNumber;
  Ptr<RequestSentences> sentences;
  PCItem() : batchNumber(-1), sentences(NULL) {}
  PCItem(int batchNumber, Ptr<RequestSentences> sentences)
      : batchNumber(batchNumber), sentences(sentences) {}

  void operator=(const PCItem &b) {
    batchNumber = b.batchNumber;
    sentences = b.sentences;
  }

  bool isPoison() { return (batchNumber == -1); }
};

} // namespace bergamot
} // namespace marian

#endif // SRC_BERGAMOT_REQUEST_H_
