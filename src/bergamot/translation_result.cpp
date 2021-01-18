#include "translation_result.h"
#include "data/alignment.h"

namespace marian {
namespace bergamot {

TranslationResult::TranslationResult(std::string &&source, Segments &&segments,
                                     SourceAlignments &&sourceAlignments,
                                     Histories &&histories,
                                     std::vector<Ptr<Vocab const>> &vocabs)
    : source_(std::move(source)),
      sourceAlignments_(std::move(sourceAlignments)),
      segments_(std::move(segments)), histories_(std::move(histories)),
      vocabs_(&vocabs) {}

string_view TranslationResult::getUnderlyingSource(int index) {
  string_view first = sourceAlignments_[index].front();
  string_view last = sourceAlignments_[index].back();
  int size = last.end() - first.begin();
  string_view sourceView(first.data(), size);
  return sourceView;
}

std::string TranslationResult::getSource(int index) {
  Words words = segments_[index];
  std::string decoded = vocabs_->front()->decode(words);
  return decoded;
}

std::string TranslationResult::getTranslation(int index) {
  Ptr<History> history = histories_[index];
  NBestList onebest = history->nBest(1);
  Result result = onebest[0]; // Expecting only one result;
  Words words = std::get<0>(result);
  std::string decoded = vocabs_->back()->decode(words);
  return decoded;
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
