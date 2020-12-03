#include "common/timer.h"
#include "common/utils.h"
#include "marian.h"
#include "translator/beam_search.h"
#include "translator/output_printer.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "textops.h"

/* Looks unnecessary
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#include "crow.h"
#pragma GCC diagnostic pop
*/

/*
 : smode_(smode) {
        size_t linectr = 0;
        std::string snt;
        auto buf = service.createSentenceStream(input, smode);
        while (buf >> snt) {
          LOG(trace, "SNT: {}", snt);
          auto foo = std::move(servkkice.push(++linectr, snt, &topts));
          pending_jobs_.push_back(std::move(foo.second));
        }
        finished_jobs_.resize(pending_jobs_.size());
        ends_with_eol_char_ = input.size() && input.back() == '\n';
      }
*/

int main(int argc, char* argv[]) {
  marian::ConfigParser cp(marian::cli::mode::translation);

  cp.addOption<int>("--port,-p", "Server Options", "server port", 18080);
  cp.addOption<int>("--queue-timeout", "Server Options",
                    "max wait time (in ms) for new data before an underfull "
                    "batch is launched",
                    100);
  cp.addOption<size_t>(
      "--max-workers", "Server Options",
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

  // TODO(jerin): Add QE later.
  // marian::qe::QualityEstimator::addOptions(cp);

  auto options = cp.parseOptions(argc, argv, true);


  const char* _smode_char = "paragraph";
  string smode_char(_smode_char);

  std::string input;
  std::getline(std::cin, input); 
  std::cout<<input<<"\n";

  auto sentence_splitter = SentenceSplitter(options);
  auto smode = sentence_splitter.string2splitmode(smode_char, false);
  auto buf = sentence_splitter.createSentenceStream(input, smode);

  std::string snt;
  while (buf >> snt) {
    LOG(trace, "SNT: {}", snt);
  }
}