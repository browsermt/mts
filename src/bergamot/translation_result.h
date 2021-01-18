#ifndef SRC_BERGAMOT_TRANSLATION_RESULT_H_
#define SRC_BERGAMOT_TRANSLATION_RESULT_H_

#include "data/types.h"
#include "definitions.h"
#include "translator/beam_search.h"

#include <cassert>
#include <string>
#include <vector>

namespace marian {
namespace bergamot {
class TranslationResult {
  // Once a request is completed, the constructs to postprocess the same is
  // moved into a translation result. The API provides a lazy means to obtain
  // 1. sourceText (raw underlying text)
  // 2. Translation
  // 3. Alignments between string_views of tokens in sourceText and Translation.

public:
  TranslationResult(std::string &&source, Segments &&segments,
                    SourceAlignments &&sourceAlignments, Histories &&histories,
                    std::vector<Ptr<Vocab const>> &vocabs);

  unsigned int numUnits() { return segments_.size(); };

  // Provides raw text before being (unicode) normalized.
  string_view getUnderlyingSource(int index);

  // Source as decoded by the vocab (normalized, unlike UnderlyingSource).
  std::string getSource(int index);

  // Translation of the unit at corresponding index.
  std::string getTranslation(int index);

  // Provides a hard alignment between source and target words.
  std::vector<int> getAlignment(int index);

private:
  std::string source_;
  Segments segments_;
  SourceAlignments sourceAlignments_;
  Histories histories_;
  std::vector<Ptr<Vocab const>> *vocabs_;
};
} // namespace bergamot
} // namespace marian

#endif // SRC_BERGAMOT_TRANSLATION_RESULT_H_
