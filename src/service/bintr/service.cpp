#include "service.h"

namespace marian {
  namespace bergamot {

    Service::Service(Ptr<Options> options) : text_processor_(options) {
      BatchTranslator batch_translator(CPU0, text_processor_.tokenizer_.vocabs_,
          options);
      batch_translator_ = batch_translator;
    }

    std::future<TranslationResult> Service::trivial_translate(string_view &input) {
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

      for (auto history : histories) {
        NBestList onebest = history->nBest(1);
        Result result = onebest[0];  // Expecting only one result;
        Words words = std::get<0>(result);
        std::string processed_sentence;
        processed_sentence =
          text_processor_.tokenizer_.vocabs_.back()->decode(words);
        translation_result.translations.push_back(processed_sentence);
      }
      translation_result_promise.set_value(translation_result);

      return translation_result_promise.get_future();
    }

    std::future<TranslationResult> Service::queue(string_view &input) {
      std::promise<TranslationResult> translation_result_promise;
      auto segments = text_processor_.query_to_segments(input);
      Ptr<Request> request = New<Request>(segments, translation_result_promise);
      return translation_result_promise.get_future();
    }

    std::future<TranslationResult> Service::translate(string_view &input) {
      return queue(input);
      // return trivial_translate(input);
    }

  }
}
