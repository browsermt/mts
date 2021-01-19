#include "translation_result.h"
#include "data/alignment.h"

#include <utility>

namespace marian {
namespace bergamot {

TranslationResult::TranslationResult(std::string &&source, Segments &&segments,
                                     SourceAlignments &&sourceAlignments,
                                     Histories &&histories,
                                     std::vector<Ptr<Vocab const>> &vocabs)
    : source_(std::move(source)),
      sourceAlignments_(std::move(sourceAlignments)),
      segments_(std::move(segments)), histories_(std::move(histories)),
      vocabs_(&vocabs) {

  // Process sourceMappings into sourceMappings_.
  sourceMappings_.reserve(segments_.size());
  for (int i = 0; i < segments_.size(); i++) {
    string_view first = sourceAlignments_[i].front();
    string_view last = sourceAlignments_[i].back();
    int size = last.end() - first.begin();
    sourceMappings_.emplace_back(first.data(), size);
  }

  // Compiles translations into a single std::string translation_
  // Current implementation uses += on std::string, multiple resizes.
  // Stores ByterRanges as indices first, followed by conversion into
  // string_views.
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
  targetMappings_.reserve(translationRanges.size());
  for (auto &p : translationRanges) {
    targetMappings_.emplace_back(&translation_[p.first], p.second - p.first);
  }
}

string_view TranslationResult::getSource(int index) const {
  return sourceMappings_[index];
}

std::string TranslationResult::getNormalizedSource(int index) const {
  Words words = segments_[index];
  std::string decoded = vocabs_->front()->decode(words);
  return decoded;
}

string_view TranslationResult::getTranslation(int index) const {
  return targetMappings_[index];
}

std::vector<int> TranslationResult::getAlignment(int index) {
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
