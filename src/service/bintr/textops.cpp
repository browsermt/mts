#include "textops.h"

#include<pcrecpp.h> // For StringPiece


namespace marian {
namespace bergamot {


SentenceSplitter::SentenceSplitter(marian::Ptr<marian::Options> options)
    : options_(options) {

  std::string smode_str = options_->get<std::string>("ssplit-mode", "");
  mode_ = string2splitmode(smode_str);
  std::string ssplit_prefix_file = options_->get<std::string>("ssplit-prefix-file", "");

  if (ssplit_prefix_file.size()) {
    ssplit_prefix_file = marian::cli::interpolateEnvVars(ssplit_prefix_file);
    LOG(info, "Loading protected prefixes for sentence splitting from {}",
        ssplit_prefix_file);
    ssplit_.load(ssplit_prefix_file);
  } else {
    LOG(warn,
        "Missing list of protected prefixes for sentence splitting. "
        "Set with --ssplit-prefix-file.");
  }
}

ug::ssplit::SentenceStream SentenceSplitter::createSentenceStream(const string_view &input) {
  pcrecpp::StringPiece spiece(input.begin(), input.size());
  return std::move(ug::ssplit::SentenceStream(spiece, this->ssplit_, mode_));
}

ug::ssplit::SentenceStream::splitmode SentenceSplitter::string2splitmode(const std::string &m) {
  typedef ug::ssplit::SentenceStream::splitmode splitmode;
  // @TODO: throw Exception on error
  if (m == "sentence" || m == "Sentence")
    return splitmode::one_sentence_per_line;
  if (m == "paragraph" || m == "Paragraph")
    return splitmode::one_paragraph_per_line;
  if (m != "wrapped_text" && m != "WrappedText" && m != "wrappedText") {
    LOG(warn, "Ignoring unknown text input format specification: {}.", m);
  }
  return splitmode::wrapped_text;
}

Tokenizer::Tokenizer(Ptr<Options> options) : inference_(true), addEOS_(false) {
  vocabs_ = loadVocabularies(options);
}

std::vector<Ptr<const Vocab>> Tokenizer::loadVocabularies(
    Ptr<Options> options) {
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

Segment Tokenizer::tokenize(string_view const &snt, std::vector<string_view> &alignments) {
  // TODO(jerin): Bunch of hardcode here, 1, 0, need to get rid off somehow.
  return vocabs_[0]->encodePreservingSource(snt, alignments, addEOS_, inference_);
}

TextProcessor::TextProcessor(Ptr<Options> options)
    : tokenizer_(options), sentence_splitter_(options) {
  max_input_sentence_tokens_ = options->get<int>("max-input-sentence-tokens");

  // Dirty assert, should do at configparse
  assert(max_input_sentence_tokens_ > 0);
}

void TextProcessor::query_to_segments(const string_view &query, 
                                      Ptr<std::vector<Segment>> segments) {
  // TODO(jerin): Paragraph is hardcoded here. Keep, looks like?
  auto buf = sentence_splitter_.createSentenceStream(query);
  pcrecpp::StringPiece snt;
  /*Ptr<std::vector<Segment>> segments = New<std::vector<Segment>>();*/
  std::vector<std::vector<string_view>> alignments;

  while (buf >> snt) {
    LOG(trace, "SNT: {}", snt);
    string_view snt_string_view(snt.data(), snt.size());
    std::vector<string_view> snt_alignments;
    Segment tokenized_sentence = tokenizer_.tokenize(snt_string_view, snt_alignments);

    if (tokenized_sentence.size() > max_input_sentence_tokens_) {
      int offset;
      for (offset = 0;
           offset + max_input_sentence_tokens_ < tokenized_sentence.size();
           offset += max_input_sentence_tokens_) {
        
        auto start = tokenized_sentence.begin() + offset;
        Segment segment(start, start + max_input_sentence_tokens_);
        segments->push_back(segment);
      }

      if (offset < max_input_sentence_tokens_) {
        auto start = tokenized_sentence.begin() + offset;
        Segment segment(start, tokenized_sentence.end());
        segments->push_back(segment);
      }

    }

    else {
      segments->push_back(tokenized_sentence);
    }

  }
}

}  // namespace bergamot
}  // namespace marian
