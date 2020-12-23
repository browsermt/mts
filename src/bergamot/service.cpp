#include "service.h"
#include <utility>
#include <string>

namespace marian {
namespace bergamot {

Service::Service(Ptr<Options> options) :
    text_processor_(options), batcher_(options) {
  BatchTranslator batch_translator(CPU0, text_processor_.tokenizer_.vocabs_,
                                   options);
  batch_translator_ = batch_translator;
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
  std::cout << "Segments proc" << std::endl;

  std::cout << "Construct batches" << std::endl;
  auto batch = batch_translator_.construct_batch_from_segments(segments);
  std::cout << "Translating" << std::endl;
  auto histories = batch_translator_.translate_batch(batch);

  std::promise<TranslationResult> translation_result_promise;
  auto translation_result = process(segments, histories);
  translation_result_promise.set_value(translation_result);
  return translation_result_promise.get_future();
}

std::future<TranslationResult> Service::queue(const string_view &input) {
  // TODO(jerin): Place a queue to keep track of requests here.
  Ptr<Segments> segments = New<Segments>();
  Ptr<Alignments> alignments = New<Alignments>();

  std::cout << "Query to segments"  <<  std::endl;
  text_processor_.query_to_segments(input, segments, alignments);

  std::promise<TranslationResult> translation_result_promise_;
  Ptr<Request> request = New<Request>(input,
                                      std::move(segments),
                                      std::move(alignments),
                                      translation_result_promise_);

  std::cout << "Queuing"  <<  std::endl;
  for (int i = 0; i < request->size(); i++) {
    MultiFactorPriority priority(i, request);
    batcher_.addSentenceWithPriority(priority);
  }

  Ptr<Segments> batchSegments = New<Segments>();
  Ptr<std::vector<MultiFactorPriority>> batchSentences 
      = New<std::vector<MultiFactorPriority>>();

  /* Cleave batch, run translation */
  batcher_.cleave_batch(batchSegments, batchSentences);
  std::cout << "Construct batches" << std::endl;
  auto batch = batch_translator_.construct_batch_from_segments(batchSegments);
  std::cout << "Translating" << std::endl;
  auto histories = batch_translator_.translate_batch(batch);

  for(int i=0; i < batchSentences->size(); i++){
    Ptr<History> history = histories[i];
    Ptr<Request> request = (batchSentences->at(i)).request;
    int index = (batchSentences->at(i)).index;
    request->set_translation(index, decode(history));
  }

  // TODO(jerin): remove.
  request->join();

  return translation_result_promise_.get_future();
}

std::future<TranslationResult> Service::translate(const string_view &input) {
  return queue(input);
  // return trivial_translate(input);
}


}  // namespace bergamot
}  // namespace marian
