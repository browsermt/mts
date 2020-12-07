#pragma once
#include "batch_translator.h"
#include "request.h"
#include "textops.h"
#include "translation_result.h"
#include "translator/beam_search.h"

namespace marian {
namespace bergamot {
class Service {
  TextProcessor text_processor_;
  BatchTranslator batch_translator_;

 public:
  Service(Ptr<Options> options) : text_processor_(options) {
    BatchTranslator batch_translator(CPU0, text_processor_.tokenizer_.vocabs_,
                                     options);
    batch_translator_ = batch_translator;
  }

  std::future<TranslationResult> trivial_translate(std::string &input) {
    /*
     * Cut somewhere here and connect to batch_translator
     *
     * Trivial translate, which Abhishek should possibly able to bring up on
     * his own.
     *
     */

    std::promise<TranslationResult> translation_result_promise;
    TranslationResult translation_result;

    auto segments = text_processor_.query_to_segments(input);
    for (auto words : segments) {
      std::string processed_sentence;
      processed_sentence =
          text_processor_.tokenizer_.vocabs_.front()->decode(words);
      translation_result.sources.push_back(processed_sentence);
    }

    auto batch = batch_translator_.construct_batch_from_segments(segments);
    auto histories =
        batch_translator_.translate_batch<Ptr<data::CorpusBatch>, BeamSearch>(
            batch);

    /* Do stuff with histories */
    for (auto history : histories) {
      NBestList onebest = history->nBest(1);
      Result result = onebest[0];  // Expecting only one result;
      Words words = std::get<0>(result);
      std::string processed_sentence;
      processed_sentence =
          text_processor_.tokenizer_.vocabs_.back()->decode(words);
      /* std::cout << processed_sentence << "\n"; */
      translation_result.translations.push_back(processed_sentence);
    }
    translation_result_promise.set_value(translation_result);
    return translation_result_promise.get_future();
  }

  std::future<TranslationResult> queue(std::string &input) {
    /* Multithreaded translate, using producer consumer and heuristic mechanism
     */

    std::promise<TranslationResult> translation_result_promise;
    auto segments = text_processor_.query_to_segments(input);

    /* TODO(jerin): Fix some hardcodes below; */
    Request request(0, segments, translation_result_promise);
    return translation_result_promise.get_future();
  }

  std::future<TranslationResult> translate(std::string &input) {
    return queue(input);
    // return trivial_translate(input);
  }
};

}  // namespace bergamot
}  // namespace marian
