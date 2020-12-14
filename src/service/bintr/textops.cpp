#include "textops.h"


#include<pcrecpp.h> // For StringPiece


namespace marian {
namespace bergamot {
SentenceSplitter::SentenceSplitter(marian::Ptr<marian::Options> options)
    : options_(options) {
  auto ssplit_prefix_file =
      options_->get<std::string>("ssplit-prefix-file", "");

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

ug::ssplit::SentenceStream SentenceSplitter::createSentenceStream(
    string_view const &input,
    ug::ssplit::SentenceStream::splitmode const &mode) {
      pcrecpp::StringPiece spiece(input.begin(), input.size());
  return std::move(ug::ssplit::SentenceStream(spiece, this->ssplit_, mode));
}

ug::ssplit::SentenceStream::splitmode SentenceSplitter::string2splitmode(
    const std::string &m, bool throwOnError /*=false*/) {
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

Tokenizer::Tokenizer(Ptr<Options> options) : inference_(true), addEos_(true) {
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
  return vocabs_[0]->encodePreservingSource(snt, alignments, false, true);
}

TextProcessor::TextProcessor(Ptr<Options> options)
    : tokenizer_(options), sentence_splitter_(options) {
  max_input_sentence_tokens_ = options->get<int>("max-input-sentence-tokens");

  // Dirty assert, should do at configparse
  assert(max_input_sentence_tokens_ > 0);
}

std::vector<Segment> TextProcessor::query_to_segments(const string_view &query) {
  // TODO(jerin): Paragraph is hardcoded here. Keep, looks like?
  auto smode = sentence_splitter_.string2splitmode("paragraph", false);
  auto buf = sentence_splitter_.createSentenceStream(query, smode);
  pcrecpp::StringPiece snt;
  std::vector<Segment> segments;
  std::vector<string_view> alignments;

  while (buf >> snt) {
    LOG(trace, "SNT: {}", snt);
    string_view snt_string_view(snt.data(), snt.size());
    Segment tokenized_sentence = tokenizer_.tokenize(snt_string_view, alignments);
    segments.push_back(tokenized_sentence);

    // Check if tokens are length enough, else break.
    /*

    if (tokenized_sentence.size() > max_input_sentence_tokens_) {
      // Cutting strategy, just cut max_input_size_tokens pieces
      int offset;
      for (offset = -1;
           offset + max_input_sentence_tokens_ < tokenized_sentence.size();
           offset += max_input_sentence_tokens_) {
        Segment segment(
            tokenized_sentence.begin() + offset,
            tokenized_sentence.begin() + offset + max_input_sentence_tokens_);
        // segments.push_back(segment);
      }

      // Once for loop is done, last bit is left.
      if (offset < max_input_sentence_tokens_) {
        Segment segment(tokenized_sentence.begin() + offset,
                      tokenized_sentence.end());
        // segments.push_back(segment);
      }

    }

    // Might be an unnecessary else, but stay for now.
    else {
      // segments.push_back(tokenized_sentence);
    }

    */
  }
  return segments;
}

}  // namespace bergamot
}  // namespace marian