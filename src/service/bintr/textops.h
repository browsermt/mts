
/*
 * Interact with sentencepiece and ssplit to preprocess or postprocess
 * sentences.
 */

#pragma once



#include "common/definitions.h"
#include "common/options.h"
#include "ssplit/ssplit.h"
// #include "data/types.h"
#include "common/definitions.h"
#include "common/logging.h"
#include "common/types.h"  // missing in shortlist.h
#include "common/utils.h"
#include "data/shortlist.h"
#include "data/sentencepiece_vocab.h"
#include "data/vocab_base.h"
#include "definitions.h"

namespace marian {
namespace bergamot {


class SentenceSplitter;

class SentenceSplitter {
  /* Using this class to hide away ssplit mechanics */
 private:
  ug::ssplit::SentenceSplitter ssplit_;
  Ptr<Options> options_;

 public:
  typedef ug::ssplit::SentenceStream::splitmode ssplitmode;

  // Constructor
  SentenceSplitter(Ptr<Options> options);

  ug::ssplit::SentenceStream createSentenceStream(
      string_view const &input,
      ug::ssplit::SentenceStream::splitmode const &mode);
  ug::ssplit::SentenceStream::splitmode string2splitmode(
      const std::string &m, bool throwOnError /*=false*/);
};

class Tokenizer {
 public:
  std::vector<Ptr<Vocab const>> vocabs_;
  bool inference_;
  bool addEos_;
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
