#include "service.h"

namespace marian {
  namespace bergamot {

    Service::Service(Ptr<Options> options) : text_processor_(options), batcher_(options) {
      BatchTranslator batch_translator(CPU0, text_processor_.tokenizer_.vocabs_,
          options);
      batch_translator_ = batch_translator;
    }

    std::future<TranslationResult> Service::trivial_translate(string_view &input) {
      std::promise<TranslationResult> translation_result_promise;
      TranslationResult translation_result;

      Ptr<Segments> segments = New<Segments>();
      Ptr<Alignments> alignments
        = New<Alignments>();
      text_processor_.query_to_segments(input, segments, alignments);
      std::cout<<"Segments proc"<<std::endl;
      for (auto words : *segments) {
        std::string processed_sentence;
        processed_sentence =
          text_processor_.tokenizer_.vocabs_.front()->decode(words);
        translation_result.sources.push_back(processed_sentence);
      }

      std::cout<<"Construct batches"<<std::endl;
      auto batch = batch_translator_.construct_batch_from_segments(segments);
      std::cout<<"Translating"<<std::endl;
      auto histories = batch_translator_.translate_batch<Ptr<data::CorpusBatch>, BeamSearch>(
            batch);

      std::cout<<"printing"<<std::endl;
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
      Ptr<Segments> segments = New<Segments>();
      Ptr<Alignments> alignments = New<Alignments>();

      std::cout<<"Query to segments" << std::endl;
      text_processor_.query_to_segments(input, segments, alignments);
      Ptr<Request> request = New<Request>(segments, 
                                          alignments, 
                                          translation_result_promise);
      std::cout<<"Queued" << std::endl;
      for(int i=0; i<segments->size(); i++){
        MultiFactorPriority priority(i, request);
        batcher_.addSentenceWithPriority(priority);
      }
      return translation_result_promise.get_future();
    }

    std::future<TranslationResult> Service::translate(string_view &input) {
      // return queue(input);
      return trivial_translate(input);
    }

  }
}
