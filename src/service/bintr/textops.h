
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

  std::vector<data::SentenceTuple> first_pass(std::string query) {
    // TODO(jerin): Paragraph is hardcoded here. Keep, looks like?
    auto smode = sentence_splitter_.string2splitmode("paragraph", false);
    auto buf = sentence_splitter_.createSentenceStream(query, smode);
    std::string snt;
    std::vector<data::SentenceTuple> sentence_tuples;

    int id = 0;

    while (buf >> snt) {
      LOG(trace, "SNT: {}", snt);
      auto tokenized_sentence = tokenizer_.tokenize(snt);

      // Check if tokens are length enough, else break.

      if(tokenized_sentence.size() > max_input_sentence_tokens_){
         // Cutting strategy, just cut max_input_size_tokens pieces
         int offset;
         for(offset=0; 
           offset+max_input_sentence_tokens_ < tokenized_sentence.size(); 
           offset+=max_input_sentence_tokens_){

           data::SentenceTuple sentence_tuple(id);
           id++;
           Words segment(tokenized_sentence.begin()+offset, 
                                tokenized_sentence.begin()+offset+max_input_sentence_tokens_);
           sentence_tuple.push_back(segment);
           sentence_tuples.push_back(sentence_tuple);
         }

         // Once for loop is done, last bit is left.
         if(offset < max_input_sentence_tokens_){
           data::SentenceTuple sentence_tuple(id);
           id++;
           Words segment(tokenized_sentence.begin()+offset, 
                                tokenized_sentence.end());
           sentence_tuple.push_back(segment);
           sentence_tuples.push_back(sentence_tuple);
         }

      }
      
      // Might be an unnecessary else, but stay for now.
      else{
        data::SentenceTuple sentence_tuple(id);
        id++;
        sentence_tuple.push_back(tokenized_sentence);
        sentence_tuples.push_back(sentence_tuple);
      }

    }
    return sentence_tuples;
  }

};

}  // namespace bergamot
}  // namespace marian
