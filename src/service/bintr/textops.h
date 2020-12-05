
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

namespace marian
{
  namespace bergamot
  {
    class SentenceSplitter;
    //class Tokenizer;

    class SentenceSplitter
    {
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

    class Tokenizer
    {
    public:
      std::vector<Ptr<Vocab const>> vocabs_;
      bool inference_;
      bool addEos_;
      Tokenizer(Ptr<Options>);
      std::vector<Ptr<const Vocab>> loadVocabularies(Ptr<Options> options);
      data::SentenceTuple tokenize(std::string const &snt);
    };

    class TextProcessor
    {
    public:
      Tokenizer tokenizer;
      SentenceSplitter sentence_splitter;
      TextProcessor(Ptr<Options>);

      std::vector<data::SentenceTuple> first_pass(std::string query)
      {
        const char *_smode_char = "paragraph";
        string smode_char(_smode_char);
        auto smode = sentence_splitter.string2splitmode(smode_char, false);
        auto buf = sentence_splitter.createSentenceStream(query, smode);
        std::string snt;
        std::vector<data::SentenceTuple> sentence_tuples;

        while (buf >> snt)
        {
          LOG(trace, "SNT: {}", snt);
          auto sentence_tuple = tokenizer.tokenize(snt);
          sentence_tuples.push_back(sentence_tuple);
        }
        return sentence_tuples;
      }
    };

  } // namespace bergamot
} // namespace marian
