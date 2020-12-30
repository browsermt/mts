#include "batch_translator.h"

namespace marian {
namespace bergamot {

BatchTranslator::BatchTranslator(DeviceId const device,
                                 std::vector<Ptr<Vocab const>> vocabs,
                                 Ptr<PCQueue<PCItem>> pcqueue,
                                 Ptr<Options> options)
    : device_(device), options_(options), vocabs_(vocabs), 
      pcqueue_(pcqueue) {

  if (options_->hasAndNotEmpty("shortlist")) {
    Ptr<data::ShortlistGenerator const> slgen;
    int srcIdx = 0, trgIdx = 1;
    bool shared_vcb = vocabs_.front() == vocabs_.back();
    slgen_ = New<data::LexicalShortlistGenerator>(
        options_, vocabs_.front(), vocabs_.back(), srcIdx, trgIdx, shared_vcb);
  }

  graph_ = New<ExpressionGraph>(true);  // always optimize
  auto prec =
      options_->get<std::vector<std::string>>("precision", {"float32"});
  graph_->setDefaultElementType(typeFromString(prec[0]));
  graph_->setDevice(device_);
  graph_->getBackend()->configureDevice(options_);
  graph_->reserveWorkspaceMB(options_->get<size_t>("workspace"));
  scorers_ = createScorers(options_);
  for (auto scorer : scorers_) {
    scorer->init(graph_);
    if (slgen_) {
      scorer->setShortlistGenerator(slgen_);
    }
  }

  graph_->forward();

  ABORT_IF(thread_ != NULL, "Don't call start on a running worker!");
  thread_.reset(new std::thread([this]{ this->mainloop(); }));

}

void BatchTranslator::translate(const Ptr<Segments> segments, 
                                Ptr<Histories> histories){
  int id = 0;
  std::vector<data::SentenceTuple> batchVector;
  for (auto &segment : *segments) {
    data::SentenceTuple sentence_tuple(id);
    sentence_tuple.push_back(segment);
    batchVector.push_back(sentence_tuple);
    id++;
  }

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

  for (size_t j = 0; j < maxDims.size(); ++j) 
    subBatches[j]->setWords(words[j]);

  auto batch = Ptr<CorpusBatch>(new CorpusBatch(subBatches));
  batch->setSentenceIds(sentenceIds);


  auto trgVocab = vocabs_.back();
  auto search = New<BeamSearch>(options_, scorers_, trgVocab);

  *histories = std::move(search->search(graph_, batch));

}

void BatchTranslator::mainloop(){
  while(true){
    PCItem pcitem;
    pcqueue_->Consume(pcitem);
    LOG(info, "Worker {} consuming item; ", device_.no);
    Ptr<Histories> histories = New<Histories>();
    translate(pcitem.segments, histories);
    for(int i=0; i < (pcitem.sentences)->size(); i++){
      Ptr<History> history = histories->at(i);
      Ptr<Request> request = ((pcitem.sentences)->at(i)).request;
      int index = ((pcitem.sentences)->at(i)).index;
      request->set_translation(index, history);
    }
    LOG(info, "Worker {} processed item; ", device_.no);
  }
}

}  // namespace bergamot
}  // namespace marian
