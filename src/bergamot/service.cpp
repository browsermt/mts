#include "service.h"
#include "utils.h"
#include <utility>
#include <string>

namespace marian {
namespace bergamot {

Service::Service(Ptr<Options> options) :
    text_processor_(options), batcher_(options), 
    timeout_(options->get<int>("queue-timeout")) {

  int num_workers = options->get<int>("cpu-threads");

  // Load vocabulary, to be shared among workers and tokenizer.
  vocabs_ = loadVocabularies(options);

  // @TODO(jerin): Fix hardcode, 100*num_workers
  pcqueue_ = New<Queue<PCItem>>(100*num_workers);

  // workers_ = New<std::vector<Ptr<BatchTranslator>>>();
  workers_.reserve(num_workers);

  for(int i=0; i < num_workers; i++){
    marian::DeviceId deviceId(i, DeviceType::cpu);
    Ptr<BatchTranslator> batch_translator
        = New<BatchTranslator>(deviceId, pcqueue_, options);
    workers_.push_back(std::move(batch_translator));
  }
}

std::future<TranslationResult> Service::queue(const string_view &input) {
  // @TODO(jerin): Place a queue to keep track of requests here.
  Ptr<Segments> segments = New<Segments>();
  Ptr<SourceAlignments> sourceAlignments = New<SourceAlignments>();
  text_processor_.query_to_segments(input, segments, sourceAlignments);

  for(auto &segment: *segments){
      PLOG("main", info, "[token-size {}]", ((int)segment.size()));
  }

  Ptr<std::promise<TranslationResult>> 
      translationResultPromise = New<std::promise<TranslationResult>>();
  auto future = translationResultPromise->get_future();

  Ptr<Request> request = New<Request>(vocabs_, 
                                      input,
                                      std::move(segments),
                                      std::move(sourceAlignments),
                                      translationResultPromise);

  for (int i = 0; i < request->size(); i++) {
    RequestSentence requestSentence(i, request);
    batcher_.addSentenceWithPriority(requestSentence);
  }

  /* Cleave batch, run translation */
  Ptr<Segments> batchSegments; 
  Ptr<RequestSentences> batchSentences;

  int counter = 0;
  do {
    // TODO(jerin): Check move under-the-hood / memory leaks.
    batchSegments = New<Segments>();
    batchSentences = New<std::vector<RequestSentence>>();
    batcher_.cleave_batch(batchSegments, batchSentences);

    if(batchSegments->size() > 0){
      PCItem pcitem(batchSegments, batchSentences);
      // pcqueue_->push(pcitem, timeout_);
      pcqueue_->push(pcitem);
      ++counter;
      PLOG("main", info, "Batch {} generated", counter);
    }
  } while (batchSegments->size() > 0);
  
  return future;

}

std::future<TranslationResult> 
Service::translate(const string_view &input) {
  return queue(input);
}

void Service::stop(){
  int counter = 0;
  for(auto &worker: workers_){
    PLOG("main", info, "Stopping worker {}", counter);
    worker->stop();
    PLOG("main", info, "Stopped worker {}", counter);
    ++counter;
  }

  counter = 0;
  for(auto &worker: workers_){
    PLOG("main", info, "Joining worker {}", counter);
    worker->join();
    PLOG("main", info, "Joining worker {}", counter);
    ++counter;
  }
}

}  // namespace bergamot
}  // namespace marian
