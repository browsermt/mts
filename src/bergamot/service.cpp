#include "service.h"
#include "definitions.h"

#include "utils.h"
#include <string>
#include <utility>

namespace marian {
namespace bergamot {

Service::Service(Ptr<Options> options)
    : text_processor_(options), batcher_(options), running_(true),
      requestId_(0), batchNumber_(0) {

  int num_workers = options->get<int>("cpu-threads");

  // Load vocabulary, to be shared among workers and tokenizer.
  vocabs_ = loadVocabularies(options);
  pcqueue_ = UNew<PCQueue<PCItem>>(2 * num_workers);
  workers_.reserve(num_workers);

  for (int i = 0; i < num_workers; i++) {
    // Initialize worker
    marian::DeviceId deviceId(i, DeviceType::cpu);
    UPtr<BatchTranslator> batch_translator =
        UNew<BatchTranslator>(deviceId, *pcqueue_.get(), options);

    // Move worker into container
    workers_.push_back(std::move(batch_translator));
  }
}

std::future<TranslationResult> Service::queue(const string_view &input) {
  // @TODO(jerin): Place a queue to keep track of requests here.

  // Create segments and sourceAlignments, load via TextProcessor
  UPtr<Segments> segments = UNew<Segments>();
  UPtr<SourceAlignments> sourceAlignments = UNew<SourceAlignments>();

  text_processor_.query_to_segments(input, *segments.get(),
                                    *sourceAlignments.get());

  // Create promise and obtain future.
  std::promise<TranslationResult> translationResultPromise;
  auto future = translationResultPromise.get_future();

  // Move ownership and construct request.
  UPtr<Request> request = UNew<Request>(
      requestId_++, vocabs_, input, std::move(segments),
      std::move(sourceAlignments), std::move(translationResultPromise));

  // Adding sentences from a request to batcher.
  for (int i = 0; i < request->numSegments(); i++) {
    RequestSentence requestSentence(i, *request.get());
    batcher_.addSentenceWithPriority(requestSentence);
  }

  // Moving request into service Ownership to keep UPtr from deleting once out
  // of scope. Alternative: Use shared_ptr? Overhead doesn't seem to be much.
  requests_.push(std::move(request));

  // Construct batches of RequestSentences and add to PCQueue for workers to
  // consume.
  UPtr<RequestSentences> batchSentences;
  int numSentences;
  do {
    batchSentences = UNew<std::vector<RequestSentence>>();
    batcher_.cleave_batch(*batchSentences.get());

    // Using batchSentences-> directly with move-semantics lead to segfault.
    numSentences = batchSentences->size();

    if (numSentences > 0) {
      PCItem pcitem(batchNumber_++, std::move(batchSentences));
      pcqueue_->ProduceSwap(pcitem);
      PLOG("main", info, "Batch {} generated", batchNumber_ - 1);
    }
  } while (numSentences > 0);

  return future;
}

std::future<TranslationResult> Service::translate(const string_view &input) {
  return queue(input);
}

void Service::stop() {
  if (running_) {
    int counter = 0;
    for (auto &worker : workers_) {
      PCItem pcitem;
      pcqueue_->ProduceSwap(pcitem);
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

    running_ = false;
  }
}

Service::~Service() { stop(); }

} // namespace bergamot
} // namespace marian
