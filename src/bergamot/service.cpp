#include "service.h"
#include <utility>
#include <string>

namespace marian {
namespace bergamot {

Service::Service(Ptr<Options> options) :
    text_processor_(options), batcher_(options) {

  int num_workers = options->get<int>("cpu-threads");

  // TODO(jerin): Fix hardcode, 10*num_workers
  pcqueue_ = New<PCQueue<PCItem>>(100*num_workers);

  workers_ = New<std::vector<Ptr<BatchTranslator>>>();
  workers_->reserve(num_workers);

  for(int i=0; i < num_workers; i++){
    marian::DeviceId deviceId(i, DeviceType::cpu);
    Ptr<BatchTranslator> batch_translator
        = New<BatchTranslator>(deviceId, 
                               text_processor_.tokenizer_.vocabs_, 
                               pcqueue_,
                               options);
    workers_->push_back(batch_translator);
  }

}

std::string Service::decode(Ptr<History> history){
    std::string processed_sentence;
    NBestList onebest = history->nBest(1);
    Result result = onebest[0];  // Expecting only one result;
    Words words = std::get<0>(result);
    processed_sentence =
        text_processor_.tokenizer_.vocabs_.back()->decode(words);
    return processed_sentence;
}

TranslationResult Service::process(Ptr<Segments> segments, Histories histories){
  TranslationResult translation_result;
  for (auto words : *segments) {
    std::string processed_sentence;
    processed_sentence =
        text_processor_.tokenizer_.vocabs_.front()->decode(words);
    translation_result.sources.push_back(processed_sentence);
  }
  std::cout << "printing" << std::endl;
  for (auto history : histories) {
    std::string processed_sentence;
    processed_sentence = decode(history);
    translation_result.translations.push_back(processed_sentence);
  }
  return translation_result;

}

std::future<TranslationResult>
Service::trivial_translate(const string_view &input) {

  Ptr<Segments> segments = New<Segments>();
  Ptr<Alignments> alignments
      = New<Alignments>();
  text_processor_.query_to_segments(input, segments, alignments);
  // Use first worker.
  auto histories = workers_->front()->translate_segments(segments);
  std::promise<TranslationResult> translation_result_promise;
  auto translation_result = process(segments, histories);
  translation_result_promise.set_value(translation_result);
  return translation_result_promise.get_future();
}

std::future<TranslationResult> Service::queue(const string_view &input) {
  // TODO(jerin): Place a queue to keep track of requests here.
  Ptr<Segments> segments = New<Segments>();
  Ptr<Alignments> alignments = New<Alignments>();

  text_processor_.query_to_segments(input, segments, alignments);

  Ptr<std::promise<TranslationResult>> translation_result_promise_ 
      = New<std::promise<TranslationResult>>();

  auto future =  translation_result_promise_->get_future();
  Ptr<Request> request = New<Request>(input,
                                      std::move(segments),
                                      std::move(alignments),
                                      translation_result_promise_);

  for (int i = 0; i < request->size(); i++) {
    MultiFactorPriority priority(i, request);
    batcher_.addSentenceWithPriority(priority);
  }

  /* Cleave batch, run translation */
  Ptr<Segments> batchSegments; 
  Ptr<std::vector<MultiFactorPriority>> batchSentences;
  int counter = 0;
  do {
    batchSegments = New<Segments>();
    batchSentences = New<std::vector<MultiFactorPriority>>();
    batcher_.cleave_batch(batchSegments, batchSentences);
    if(batchSegments->size() > 0){
      PCItem pcitem(batchSegments, batchSentences);
      pcqueue_->Produce(pcitem);
      ++counter;
      LOG(info, "Batch {} generated", counter);
    }
  } while (batchSegments->size() > 0);
  
  return future;

}

std::future<TranslationResult> Service::translate(const string_view &input) {
  return queue(input);
  // return trivial_translate(input);
}


}  // namespace bergamot
}  // namespace marian
