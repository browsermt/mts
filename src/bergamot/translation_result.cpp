#include "translation_result.h"
#include "common/logging.h"
#include "data/alignment.h"

#include <utility>

namespace marian {
namespace bergamot {

TranslationResult::TranslationResult(std::string &&source, Segments &&segments,
                                     std::vector<TokenRanges> &&sourceRanges,
                                     Histories &&histories,
                                     std::vector<Ptr<Vocab const>> &vocabs)
    : source_(std::move(source)), sourceRanges_(std::move(sourceRanges)),
      segments_(std::move(segments)), histories_(std::move(histories)),
      vocabs_(&vocabs) {

  // Process sourceMappings into sourceMappings_.
  LOG(info, "Creating sourcemappings");
  sourceMappings_.reserve(segments_.size());
  for (int i = 0; i < segments_.size(); i++) {
    string_view first = sourceRanges_[i].front();
    string_view last = sourceRanges_[i].back();
    int size = last.end() - first.begin();
    sourceMappings_.emplace_back(first.data(), size);
  }

  // Compiles translations into a single std::string translation_
  // Current implementation uses += on std::string, multiple resizes.
  // Stores ByterRanges as indices first, followed by conversion into
  // string_views.
  // TODO(jerin): Add token level string_views here as well.
  LOG(info, "Decoding");
  std::vector<std::pair<int, int>> translationRanges;
  int offset{0}, end{0};
  bool first{true};
  for (auto &history : histories_) {
    // TODO(jerin): Change hardcode of nBest = 1
    NBestList onebest = history->nBest(1);

    Result result = onebest[0]; // Expecting only one result;
    Words words = std::get<0>(result);
    std::string decoded = vocabs_->back()->decode(words);
    if (first) {
      first = false;
    } else {
      translation_ += " ";
    }

    translation_ += decoded;
    end = offset + (first ? 0 : 1) /*space*/ + decoded.size();
    translationRanges.emplace_back(offset, end);
    offset = end;
  }

  // Converting ByteRanges as indices into string_views.
  LOG(info, "generating targetMappings");
  targetMappings_.reserve(translationRanges.size());
  for (auto &p : translationRanges) {
    targetMappings_.emplace_back(&translation_[p.first], p.second - p.first);
  }

  // Surely, let's add sentenceMappings_
  LOG(info, "generating SentenceMappings");
  for (auto p = sourceMappings_.begin(), q = targetMappings_.begin();
       p != sourceMappings_.end() && q != targetMappings_.end(); ++p, ++q) {

    sentenceMappings_.emplace_back(*p, *q);
  }
}

/*
const string_view &TranslationResult::getSource(unsigned int index) const {
  return sourceMappings_[index];
}

std::string TranslationResult::getNormalizedSource(unsigned int index) const {
  Words words = segments_[index];
  std::string decoded = vocabs_->front()->decode(words);
  return decoded;
}

const string_view &TranslationResult::getTranslation(unsigned int index) const {
  return targetMappings_[index];
}
*/

const Histories &TranslationResult::getHistories() const { return histories_; }

std::vector<int> TranslationResult::getAlignment(unsigned int index) {
  Ptr<History> history = histories_[index];
  NBestList onebest = history->nBest(1);
  Result &result = onebest[0]; // Expecting only one result;
  Words &words = std::get<0>(result);
  auto &hypothesis = std::get<1>(result);

  // soft alignment = P(src pos|trg pos) for each beam and batch index, stored
  // in a flattened CPU-side array
  //
  // Also used on QuickSAND boundary where beam and batch size is 1. Then it is
  // simply [t][s] -> P(s|t)
  //
  // typedef std::vector<std::vector<float>> SoftAlignment;
  // [trg pos][beam depth * max src length * batch size]

  auto softAlignment = hypothesis->tracebackAlignment();
  auto hardAlignment = data::ConvertSoftAlignToHardAlign(softAlignment);
  std::vector<int> alignment(words.size(), -1);
  for (auto &p : hardAlignment) {
    alignment[p.tgtPos] = p.srcPos;
  }
  return alignment;
}

} // namespace bergamot
} // namespace marian
