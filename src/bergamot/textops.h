#ifndef __BERGAMOT_TEXTOPS_H
#define __BERGAMOT_TEXTOPS_H

#include "common/definitions.h"
#include "common/logging.h"
#include "common/options.h"
#include "common/types.h" // missing in shortlist.h
#include "common/utils.h"
#include "data/sentencepiece_vocab.h"
#include "data/shortlist.h"
#include "definitions.h"
#include "ssplit/ssplit.h"

#include <string>
#include <vector>

namespace marian {
namespace bergamot {

class SentenceSplitter {
public:
  explicit SentenceSplitter(Ptr<Options> options);
  ug::ssplit::SentenceStream createSentenceStream(string_view const &input);

private:
  ug::ssplit::SentenceSplitter ssplit_;
  Ptr<Options> options_;
  ug::ssplit::SentenceStream::splitmode mode_;
  ug::ssplit::SentenceStream::splitmode string2splitmode(const std::string &m);
};

class Tokenizer {
public:
  std::vector<Ptr<Vocab const>> vocabs_;
  bool inference_;
  bool addEOS_;
  explicit Tokenizer(Ptr<Options>);
  Segment tokenize(string_view const &, SourceAlignment &);
};

class TextProcessor {
public:
  Tokenizer tokenizer_;
  unsigned int max_input_sentence_tokens_;
  SentenceSplitter sentence_splitter_;
  explicit TextProcessor(Ptr<Options>);
  void query_to_segments(const string_view &query, Segments &,
                         SourceAlignments &);
};

} // namespace bergamot
} // namespace marian

#endif // __BERGAMOT_TEXTOPS_H
