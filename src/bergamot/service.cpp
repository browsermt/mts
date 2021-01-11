#include "service.h"

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

  // @TODO(jerin): Fix hardcode, 100*num_workers
  // @TODO(jerin): make_unique or UNew instead
  pcqueue_ = UPtr<PCQueue<PCItem>>(new PCQueue<PCItem>(100 * num_workers));

  workers_.reserve(num_workers);

  for (int i = 0; i < num_workers; i++) {
    marian::DeviceId deviceId(i, DeviceType::cpu);

    UPtr<BatchTranslator> batch_translator = UPtr<BatchTranslator>(
        new BatchTranslator(deviceId, pcqueue_.get(), options));

    workers_.push_back(std::move(batch_translator));
  }
}

std::future<TranslationResult> Service::queue(const string_view &input) {
  // @TODO(jerin): Place a queue to keep track of requests here.

  Ptr<Segments> segments = New<Segments>();
  Ptr<SourceAlignments> sourceAlignments = New<SourceAlignments>();
  text_processor_.query_to_segments(input, segments, sourceAlignments);

  Ptr<std::promise<TranslationResult>> translationResultPromise =
      New<std::promise<TranslationResult>>();
  auto future = translationResultPromise->get_future();

  Ptr<Request> request =
      New<Request>(requestId_++, vocabs_, input, std::move(segments),
                   std::move(sourceAlignments), translationResultPromise);

  for (int i = 0; i < request->numSegments(); i++) {
    RequestSentence requestSentence(i, request);
    batcher_.addSentenceWithPriority(requestSentence);
  }

  Ptr<RequestSentences> batchSentences;

  do {
    // TODO(jerin): Check move under-the-hood / memory leaks.
    batchSentences = New<std::vector<RequestSentence>>();
    batcher_.cleave_batch(batchSentences);

    if (batchSentences->size() > 0) {
      PCItem pcitem(batchNumber_++, batchSentences);
      pcqueue_->Produce(pcitem);
      PLOG("main", info, "Batch {} generated", batchNumber_ - 1);
    }
  } while (batchSentences->size() > 0);

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
      pcqueue_->Produce(pcitem);
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
