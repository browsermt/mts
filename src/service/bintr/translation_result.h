#pragma once

namespace marian {
namespace bergamot {
struct TranslationResult {
  /* A simple TranslationResult;
   * To be modified in the future with all sorts of
   * complications */
  std::vector<std::string> sources;
  std::vector<std::string> translations;
};
}  // namespace bergamot
}  // namespace marian