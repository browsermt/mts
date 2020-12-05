
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
      std::string const &input,
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
  Words tokenize(std::string const &snt);
};

class TextProcessor {
 public:
  Tokenizer tokenizer_;
  unsigned int max_input_sentence_tokens_;
  SentenceSplitter sentence_splitter_;
  TextProcessor(Ptr<Options>);

  std::vector<data::SentenceTuple> first_pass(std::string &query);
};

}  // namespace bergamot
}  // namespace marian
