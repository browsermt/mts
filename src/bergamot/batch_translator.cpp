#include "batch_translator.h"

namespace marian {
namespace bergamot {

BatchTranslator::BatchTranslator(DeviceId const device,
                                 std::vector<Ptr<Vocab const>> vocabs,
                                 Ptr<Options> options)
    : device_(device), options_(options), vocabs_(vocabs) {
  if (options_->hasAndNotEmpty("shortlist")) {
    Ptr<data::ShortlistGenerator const> slgen;
    int srcIdx = 0, trgIdx = 1;
    bool shared_vcb = vocabs_.front() == vocabs_.back();
    slgen_ = New<data::LexicalShortlistGenerator>(
        options_, vocabs_.front(), vocabs_.back(), srcIdx, trgIdx, shared_vcb);
  }
}

Ptr<data::CorpusBatch> BatchTranslator::construct_batch_from_segments(
    const Ptr<Segments> segments) {
  int id = 0;
  std::vector<data::SentenceTuple> sentence_tuples;
  for (auto &segment : *segments) {
    data::SentenceTuple sentence_tuple(id);
    sentence_tuple.push_back(segment);
    sentence_tuples.push_back(sentence_tuple);
    id++;
  }
  return construct_batch(sentence_tuples);
}

Ptr<data::CorpusBatch> BatchTranslator::construct_batch(
    const std::vector<data::SentenceTuple> &batchVector) {
  // TODO(jerin): The following is to fix rest, take off, absolute danger.
  size_t batchSize = batchVector.size();

  std::vector<size_t> sentenceIds;

  std::vector<int> maxDims;
  for (auto &ex : batchVector) {
    if (maxDims.size() < ex.size()) maxDims.resize(ex.size(), 0);
    for (size_t i = 0; i < ex.size(); ++i) {
      if (ex[i].size() > (size_t)maxDims[i]) maxDims[i] = (int)ex[i].size();
    }
    sentenceIds.push_back(ex.getId());
  }

  typedef marian::data::SubBatch SubBatch;
  typedef marian::data::CorpusBatch CorpusBatch;

  std::vector<Ptr<SubBatch>> subBatches;
  for (size_t j = 0; j < maxDims.size(); ++j) {
    subBatches.emplace_back(New<SubBatch>(batchSize, maxDims[j], vocabs_[j]));
  }

  std::vector<size_t> words(maxDims.size(), 0);
  for (size_t i = 0; i < batchSize; ++i) {
    for (size_t j = 0; j < maxDims.size(); ++j) {
      for (size_t k = 0; k < batchVector[i][j].size(); ++k) {
        subBatches[j]->data()[k * batchSize + i] = batchVector[i][j][k];
        subBatches[j]->mask()[k * batchSize + i] = 1.f;
        words[j]++;
      }
    }
  }

  for (size_t j = 0; j < maxDims.size(); ++j) subBatches[j]->setWords(words[j]);

  auto batch = Ptr<CorpusBatch>(new CorpusBatch(subBatches));
  batch->setSentenceIds(sentenceIds);
  return batch;
}

Histories BatchTranslator::translate_batch(Ptr<data::CorpusBatch> batch) {
  /* @brief
   * Run a translation on a batch and issues a callback on each of the
   * history.
   */

  // TODO(jerin): Confirm The below is one-off, or all time?
  graph_ = New<ExpressionGraph>(true);  // always optimize
  auto prec =
      options_->get<std::vector<std::string>>("precision", {"float32"});
  graph_->setDefaultElementType(typeFromString(prec[0]));
  graph_->setDevice(device_);
  graph_->getBackend()->configureDevice(options_);
  graph_->reserveWorkspaceMB(options_->get<size_t>("workspace"));
  scorers_ = createScorers(options_);
  for (auto scorer : scorers_) {
    // Why aren't these steps part of createScorers?
    // i.e., createScorers(options_, graph_, shortlistGenerator_) [UG]
    scorer->init(graph_);
    if (slgen_) {
      scorer->setShortlistGenerator(slgen_);
    }
  }
  graph_->forward();
  // Is there a particular reason that graph_->forward() happens after
  // initialization of the scorers? It would improve code readability
  // to do this before scorer initialization. Logical flow: first
  // set up graph, then set up scorers. [UG]

  /* Found bundle, callback/history. */

  auto trgVocab = vocabs_.back();
  auto search = New<BeamSearch>(options_, scorers_, trgVocab);

  // The below repeated for a batch?
  auto histories = search->search(graph_, batch);
  return histories;
}

}  // namespace bergamot
}  // namespace marian
