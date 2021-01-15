#include <cstdlib>
#include <iostream>
#include <sstream>

#include "common/definitions.h"
#include "common/timer.h"
#include "common/utils.h"
#include "marian.h"
#include "translator/beam_search.h"
#include "translator/history.h"
#include "translator/output_printer.h"

#include "service.h"

int main(int argc, char *argv[]) {
  marian::ConfigParser cp(marian::cli::mode::translation);

  cp.addOption<std::string>(
      "--ssplit-prefix-file", "Bergamot Options",
      "File with nonbreaking prefixes for sentence splitting.");

  cp.addOption<std::string>("--ssplit-mode", "Server Options",
                            "[paragraph, sentence, wrapped_text]");

  cp.addOption<int>(
      "--max-input-sentence-tokens", "Bergamot Options",
      "Maximum input tokens to be processed in a single sentence.", 128);

  cp.addOption<int>("--max-input-tokens", "Bergamot Options",
                    "Maximum input tokens in a batch. control for"
                    "Bergamot Queue",
                    1024);

  cp.addOption<int>("--nbest", "Bergamot Options",
                    "NBest value used for decoding", 1);

  // TODO(jerin): Add QE later.
  // marian::qe::QualityEstimator::addOptions(cp);

  auto options = cp.parseOptions(argc, argv, true);

  // Scan a paragraph, queue it.
  // std::string input;
  // std::getline(std::cin, input);
  // std::cout << input << "\n";

  std::ostringstream std_input;
  std_input << std::cin.rdbuf();
  std::string input = std_input.str();

  // marian::string_view input_view(input);

  marian::bergamot::Service service(options);
  auto translation_result_future = service.translate(input);
  translation_result_future.wait();
  auto translation_result = translation_result_future.get();
  for (int i = 0; i < translation_result.numUnits(); i++) {
    std::cout << "[src] " << translation_result.getUnderlyingSource(i) << "\n";
    std::cout << "[tgt] " << translation_result.getTranslation(i) << "\n";
    std::cout << "--------------------------------\n";
  }

  service.stop();
  return 0;
}
