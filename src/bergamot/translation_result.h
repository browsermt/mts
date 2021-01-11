#ifndef SRC_BERGAMOT_TRANSLATION_RESULT_H_
#define SRC_BERGAMOT_TRANSLATION_RESULT_H_

#include <vector>
#include <string>

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

#endif  // SRC_BERGAMOT_TRANSLATION_RESULT_H_
