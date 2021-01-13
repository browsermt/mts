#include "service.h"
#include "definitions.h"

#include "utils.h"
#include <string>
#include <utility>

namespace marian {
namespace bergamot {

Service::Service(Ptr<Options> options)
    : text_processor_(options), batcher_(options), requestId_(0),
      batchNumber_(0), numWorkers_(options->get<int>("cpu-threads")),
      pcqueue_(2 * numWorkers_) {

  // Load vocabulary, to be shared among workers and tokenizer.
  vocabs_ = loadVocabularies(options);
  workers_.reserve(numWorkers_);

  for (int i = 0; i < numWorkers_; i++) {
    // Initialize worker
    marian::DeviceId deviceId(i, DeviceType::cpu);
    UPtr<BatchTranslator> batch_translator =
        UNew<BatchTranslator>(deviceId, pcqueue_, options);

    // Move worker into container
    workers_.push_back(std::move(batch_translator));
  }
}

std::future<TranslationResult> Service::queue(const string_view &input) {
  Segments segments;
  SourceAlignments sourceAlignments;
  text_processor_.query_to_segments(input, segments, sourceAlignments);

  std::promise<TranslationResult> translationResultPromise;
  auto future = translationResultPromise.get_future();

  Ptr<Request> request = New<Request>(
      requestId_++, vocabs_, input, std::move(segments),
      std::move(sourceAlignments), std::move(translationResultPromise));

  for (int i = 0; i < request->numSegments(); i++) {
    RequestSentence requestSentence(i, request);
    batcher_.addSentenceWithPriority(requestSentence);
  }

  int numSentences;
  do {
    RequestSentences batchSentences;
    batcher_.cleave_batch(batchSentences);
    numSentences = batchSentences.size();

    if (numSentences > 0) {
      PCItem pcitem(batchNumber_++, std::move(batchSentences));
      PLOG("main", info, "Batch {} generating", batchNumber_ - 1);
      pcqueue_.ProduceSwap(pcitem);
      PLOG("main", info, "Batch {} generated", batchNumber_ - 1);
    }
  } while (numSentences > 0);

  return future;
}

std::future<TranslationResult> Service::translate(const string_view &input) {
  return queue(input);
}

void Service::stop() {
  int counter = 0;
  for (auto &worker : workers_) {
    PCItem pcitem;
    pcqueue_.ProduceSwap(pcitem);
    PLOG("main", info, "Adding poison {}", counter);
    ++counter;
  }

  counter = 0;
  for (auto &worker : workers_) {
    PLOG("main", info, "Joining worker {}", counter);
    worker->join();
    PLOG("main", info, "Joined worker {}", counter);
    ++counter;
  }

  workers_.clear(); // Takes care of idempotency.
}

Service::~Service() { stop(); }

} // namespace bergamot
} // namespace marian
