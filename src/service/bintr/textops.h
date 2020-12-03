
/*
 * Interact with sentencepiece and ssplit to preprocess or postprocess
 * sentences.
 */

#pragma once

#include "ssplit/ssplit.h"
#include "common/options.h"
#include "common/definitions.h"
// #include "data/types.h"
#include "common/types.h" // missing in shortlist.h
#include "common/logging.h"
#include "common/definitions.h"
#include "common/utils.h"
#include "data/shortlist.h"

class SentenceSplitter;
//class Tokenizer;

class SentenceSplitter {
  /* Using this class to hide away ssplit mechanics */
 private:
  ug::ssplit::SentenceSplitter ssplit_;
  marian::Ptr<marian::Options> options_;

 public:
  typedef ug::ssplit::SentenceStream::splitmode ssplitmode;

  // Constructor
  SentenceSplitter(marian::Ptr<marian::Options> options);

  ug::ssplit::SentenceStream createSentenceStream(
      std::string const& input,
      ug::ssplit::SentenceStream::splitmode const& mode);
  ug::ssplit::SentenceStream::splitmode string2splitmode(
      const std::string& m, bool throwOnError /*=false*/);
};

/*
class Tokenizer {
  std::vector<Ptr<Vocab const>> vocabs_;
  std::vector<Ptr<const Vocab>> loadVocabularies(Ptr<Options> options) {
    // @TODO: parallelize vocab loading for faster startup
    auto vfiles = options->get<std::vector<std::string>>("vocabs");
    // with the current setup, we need at least two vocabs: src and trg
    ABORT_IF(vfiles.size() < 2, "Insufficient number of vocabularies.");
    std::vector<Ptr<Vocab const>> vocabs(vfiles.size());
    std::unordered_map<std::string, Ptr<Vocab>> vmap;
    for (size_t i = 0; i < vocabs.size(); ++i) {
      auto m = vmap.emplace(std::make_pair(vfiles[i], Ptr<Vocab>()));
      if (m.second) {  // new: load the vocab
        m.first->second = New<Vocab>(options, i);
        m.first->second->load(vfiles[i]);
      }
      vocabs[i] = m.first->second;
    }
    return vocabs;
  }

  Tokenizer(Ptr<Options> options){
    vocabs_ = loadVocabularies(options);
  }
};

class TextProcessor {
  TextProcessor(Tokenizer, SentenceSplitter);
};
*/