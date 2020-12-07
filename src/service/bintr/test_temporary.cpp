#include <cstdlib>
#include <iostream>
#include <sstream>

#include "common/definitions.h"
#include "common/timer.h"
#include "common/utils.h"
#include "marian.h"
#include "textops.h"
#include "translator/beam_search.h"
#include "translator/history.h"
#include "translator/output_printer.h"
#include "service.h"


int main(int argc, char *argv[]) {
  marian::ConfigParser cp(marian::cli::mode::translation);

  cp.addOption<int>("--port,-p", "Server Options", "server port", 18080);
  cp.addOption<int>("--queue-timeout", "Server Options",
                    "max wait time (in ms) for new data before an underfull "
                    "batch is launched",
                    100);
  cp.addOption<size_t>(
      "--max-workers", "Bergamot Options",
      "Maximum number of worker threads to deploy when using CPU.",
      std::thread::hardware_concurrency());
  cp.addOption<std::string>("--server-root", "Server Options",
                            "server's document root directory",
                            "${HOME}/marian/lib/ui/");
  cp.addOption<std::string>(
      "--ssplit-prefix-file", "Server Options",
      "File with nonbreaking prefixes for sentence splitting.");
  cp.addOption<std::string>("--source-language", "Server Options",
                            "source language of translation service");
  cp.addOption<std::string>("--target-language", "Server Options",
                            "target language of translation service");

  cp.addOption<int>(
      "--max-input-sentence-tokens", "Bergamot Options",
      "Maximum input tokens to be processed in a single sentence.", 128);

  cp.addOption<int>("--max-input-tokens", "Bergamot Options",
                    "Maximum input tokens in a batch. control for"
                    "Bergamot Queue",
                    1024);

  // TODO(jerin): Add QE later.
  // marian::qe::QualityEstimator::addOptions(cp);

  auto options = cp.parseOptions(argc, argv, true);

  // Scan a paragraph, queue it.
  std::string input;
  std::getline(std::cin, input);
  std::cout << input << "\n";

  marian::bergamot::Service service(options);
  auto translation_result_promise = service.translate(input);
  auto translation_result = translation_result_promise.get_future().get();
  for (int i=0; i < translation_result.sources.size(); i++){
    std::cout<< "[src] " << translation_result.sources[i]<<"\n";
    std::cout<< "[tgt] " << translation_result.translations[i]<<"\n";
    std::cout<< "--------------------------------\n";
  }

  /*
  auto segments = text_proc.query_to_segments(input);
  for (auto words : segments) {
    std::string processed_sentence;
    processed_sentence = text_proc.tokenizer_.vocabs_.front()->decode(words);
    std::cout << processed_sentence << "\n";
  }

  marian::bergamot::BatchTranslator batch_translator(
      marian::CPU0, text_proc.tokenizer_.vocabs_, options);
  auto batch = batch_translator.construct_batch_from_segments(segments);
  auto histories =
      batch_translator.translate_batch<marian::Ptr<marian::data::CorpusBatch>,
                                       marian::BeamSearch>(batch);
  for (auto history : histories) {
    marian::NBestList onebest = history->nBest(1);
    marian::Result result = onebest[0];  // Expecting only one result;
    auto words = std::get<0>(result);
    std::string processed_sentence;
    processed_sentence = text_proc.tokenizer_.vocabs_.back()->decode(words);
    std::cout << processed_sentence << "\n";
  }
  */
  return 0;
}