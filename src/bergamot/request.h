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
// RequestSentence: is a tuple of (index, Request*). This provides the
// batching mechanism access to the segment within the request. The backref to
// Request allows event triggering the barrier upon completion of the last
// sentence by a worker.
//
// PCItem: is a vector of RequestSentences and a batchNumber, which is what the
// PCQueue holds. The batches are constructed from segments returned by a
// RequestSentence. Can be enhanced with paddingSize, countTokens eventually for
// logging.

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
  std::string source_;
  std::atomic<int> counter_;
  std::vector<Ptr<Vocab const>> *vocabs_;

  Segments segments_;
  SourceAlignments sourceAlignments_;
  std::vector<Ptr<History>> histories_;

  std::promise<TranslationResult> response_;

public:
  Request(unsigned int, std::vector<Ptr<Vocab const>> &, std::string &&,
          Segments &&segments, SourceAlignments &&sourceAlignments,
          std::promise<TranslationResult> translationResultPromise);

  // Obtain the count of tokens in the segment correponding to index. Used to
  // insert sentence from multiple requests into the corresponding size bucket.
  int segmentTokens(int index) const;

  // Obtain number of segments in a request.
  int numSegments() const;

  // Obtains segment corresponding to index  to create a batch of segments among
  // several requests.
  Segment getSegment(int index) const;

  // For notions of priority among requests (used to enable <set> in Batcher).
  bool operator<(const Request &request) const;

  // Processes a history obtained after translating in a heterogenous batch
  // compiled from requests.
  void processHistory(int index, Ptr<History> history);

  // On completion of last segment, sets value of the promise.
  void completeRequest();
};

class RequestSentence {
private:
  int index_;
  Ptr<Request> request_;

public:
  RequestSentence(int, Ptr<Request>);

  // Returns token in Segment corresponding to index.
  int numTokens();
  Segment getUnderlyingSegment() const;
  void completeSentence(Ptr<History> history);
  friend bool operator<(const RequestSentence &a, const RequestSentence &b);
};

typedef std::vector<RequestSentence> RequestSentences;

struct PCItem {
  int batchNumber;
  RequestSentences sentences;

  // PCItem should be default constructible for PCQueue. Default constructed
  // element is poison.
  PCItem() : batchNumber(-1) {}

  // PCItem constructor to construct a legit PCItem.
  explicit PCItem(int batchNumber, RequestSentences &&sentences)
      : batchNumber(batchNumber), sentences(std::move(sentences)) {}

  // Convenience function to determine poison.
  bool isPoison() { return (batchNumber == -1); }
};

} // namespace bergamot
} // namespace marian

#endif // SRC_BERGAMOT_REQUEST_H_
