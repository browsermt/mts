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
  typedef std::vector<std::pair<string_view, string_view>> SentenceMappings;

  TranslationResult(std::string &&source, Segments &&segments,
                    std::vector<TokenRanges> &&sourceRanges,
                    Histories &&histories,
                    std::vector<Ptr<Vocab const>> &vocabs);

  unsigned int numUnits() { return segments_.size(); };

  const string_view &getSource(unsigned int index) const;
  const string_view &getTranslation(unsigned int index) const;
  std::string getNormalizedSource(unsigned int index) const;
  const History &getHistory(unsigned int index) const;

  // Provides a hard alignment between source and target words.
  std::vector<int> getAlignment(unsigned int index);

  /* Return the original text. */
  const std::string &getOriginalText() const { return source_; }

  /* Return the translated text. */
  const std::string &getTranslatedText() const { return translation_; }

  /* Return the Quality scores of the translated text. */
  /* Not implemented, commented out.
  const QualityScore &getQualityScore() const { return qualityScore; }
  */

  const SentenceMappings &getSentenceMappings() const {
    return sentenceMappings_;
  }

private:
  std::string source_;
  std::string translation_;

  // Histories are currently required for interoperability with OutputPrinter
  // and OutputCollector and hence comparisons with marian-decoder.
  Histories histories_;

  // Not needed anymore.
  Segments segments_;
  std::vector<Ptr<Vocab const>> *vocabs_;

  // string_views at the token level.
  std::vector<TokenRanges> sourceRanges_;

  // string_views at the sentence-level.
  std::vector<string_view> sourceMappings_;
  std::vector<string_view> targetMappings_;

  // Adding the following to complete bergamot-translator spec, redundant with
  // sourceMappings_ and targetMappings_.
  SentenceMappings sentenceMappings_;
};
} // namespace bergamot
} // namespace marian

#endif // SRC_BERGAMOT_TRANSLATION_RESULT_H_
