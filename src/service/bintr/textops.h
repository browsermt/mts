#pragma once

#include "common/definitions.h"
#include "common/options.h"
#include "ssplit/ssplit.h"
#include "common/logging.h"
#include "common/types.h"  // missing in shortlist.h
#include "common/utils.h"
#include "data/shortlist.h"
#include "data/sentencepiece_vocab.h"
#include "definitions.h"

namespace marian {
namespace bergamot {


class SentenceSplitter {
  /* Using this class to hide away ssplit mechanics */
 private:
  ug::ssplit::SentenceSplitter ssplit_;
  Ptr<Options> options_;
  ug::ssplit::SentenceStream::splitmode mode_;
  ug::ssplit::SentenceStream::splitmode string2splitmode(const std::string &m);

 public:

  // Constructor
  SentenceSplitter(Ptr<Options> options);
  ug::ssplit::SentenceStream createSentenceStream(string_view const &input);
};


class Tokenizer {
 public:
  std::vector<Ptr<Vocab const>> vocabs_;
  bool inference_;
  bool addEOS_;
  Tokenizer(Ptr<Options>);
  std::vector<Ptr<const Vocab>> loadVocabularies(Ptr<Options> options);
  Segment tokenize(string_view const &snt, std::vector<string_view> &alignments);
};

class TextProcessor {
 public:
  Tokenizer tokenizer_;
  unsigned int max_input_sentence_tokens_;
  SentenceSplitter sentence_splitter_;
  TextProcessor(Ptr<Options>);

  std::vector<Segment> query_to_segments(const string_view &query);

};

}  // namespace bergamot
}  // namespace marian
