#include "service.h"
#include "definitions.h"
#include "sanelogging.h"

#include "utils.h"
#include <string>
#include <utility>

namespace marian {
namespace bergamot {

Service::Service(Ptr<Options> options)
    : requestId_(0), batchNumber_(0),
      numWorkers_(options->get<int>("cpu-threads")), text_processor_(options),
      batcher_(options), pcqueue_(2 * options->get<int>("cpu-threads")) {

  vocabs_ = loadVocabularies(options);
  workers_.reserve(numWorkers_);

  for (int i = 0; i < numWorkers_; i++) {
    marian::DeviceId deviceId(i, DeviceType::cpu);
    workers_.emplace_back(deviceId, pcqueue_, options);
  }
}

std::future<TranslationResult> Service::translate(string input) {
  // Takes in a blob of text. Segments and SourceAlignments are extracted from
  // the input (blob of text) and used to construct a Request along with a
  // promise. promise value is set by the worker completing a request.
  //
  // Batcher, which currently runs on the main thread constructs batches out of
  // a single request (at the moment) and adds them into a Producer-Consumer
  // queue holding a bunch of requestSentences used to construct batches.
  // TODO(jerin): Make asynchronous and compile from multiple requests.
  //
  // returns future corresponding to the promise.

  Segments segments;
  SourceAlignments sourceAlignments;
  text_processor_.query_to_segments(input, segments, sourceAlignments);

  std::promise<TranslationResult> translationResultPromise;
  auto future = translationResultPromise.get_future();

  Ptr<Request> request = New<Request>(
      requestId_++, vocabs_, std::move(input), std::move(segments),
      std::move(sourceAlignments), std::move(translationResultPromise));

  for (int i = 0; i < request->numSegments(); i++) {
    RequestSentence requestSentence(i, request);
    batcher_.addSentenceWithPriority(requestSentence);
  }

  int numSentences;
  do {
    RequestSentences batchSentences;
    batcher_.cleaveBatch(batchSentences);
    numSentences = batchSentences.size();

    if (numSentences > 0) {
      PCItem pcitem(batchNumber_++, std::move(batchSentences));
      pcqueue_.ProduceSwap(pcitem);
    }
  } while (numSentences > 0);

  return future;
}

void Service::stop() {
  int counter = 0;
  for (auto &worker : workers_) {
    PCItem pcitem;
    pcqueue_.ProduceSwap(pcitem);
    ++counter;
  }

  counter = 0;
  for (auto &worker : workers_) {
    worker.join();
    ++counter;
  }

  workers_.clear(); // Takes care of idempotency.
}

Service::~Service() { stop(); }

} // namespace bergamot
} // namespace marian
