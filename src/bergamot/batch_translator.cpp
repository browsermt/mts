#include "batch_translator.h"
#include "timer.h"
#include "utils.h"

namespace marian {
namespace bergamot {

BatchTranslator::BatchTranslator(DeviceId const device,
                                 Ptr<PCQueue<PCItem>> pcqueue,
                                 Ptr<Options> options)
    : device_(device), options_(options), pcqueue_(pcqueue) {



  ABORT_IF(thread_ != NULL, "Don't call start on a running worker!");
  thread_.reset(new std::thread([this]{ this->mainloop(); }));

}

void BatchTranslator::initGraph(){
  vocabs_ = loadVocabularies(options_);
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
}

void BatchTranslator::translate(const Ptr<Segments> segments, 
                                Ptr<Histories> histories){
  int id = 0;
  std::vector<data::SentenceTuple> batchVector;
  Timer timer;

  for (auto &segment : *segments) {
    data::SentenceTuple sentence_tuple(id);
    sentence_tuple.push_back(segment);
    batchVector.push_back(sentence_tuple);
    id++;
  }

  PLOG(_identifier(), info, "batchVector created in {}; ", timer.elapsed());
  timer.reset();
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


  PLOG(_identifier(), info, "subBatches created in {}; ", timer.elapsed());
  timer.reset();

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
  PLOG(_identifier(), info, "corpusBatch created in {}; ", timer.elapsed());
  timer.reset();



  auto trgVocab = vocabs_.back();
  auto search = New<BeamSearch>(options_, scorers_, trgVocab);
  auto histories_ = search->search(graph_, batch);
  PLOG(_identifier(), info, "BeamSearch completed in {}; ", timer.elapsed());

  *histories = std::move(histories_);
  timer.reset();

}

void BatchTranslator::mainloop(){
  initGraph();
  while(running_){
    Timer timer;
    PCItem pcitem;
    pcqueue_->Consume(pcitem);
    if(pcitem.isPoison()){
        running_ = false;
    } else {
      PLOG(_identifier(), info, "consumed item in {}; ", timer.elapsed());
      timer.reset();
      Ptr<Histories> histories = New<Histories>();
      translate(pcitem.segments, histories);
      PLOG(_identifier(), info, "translated item in {}; ", timer.elapsed());
      timer.reset();
      for(int i=0; i < (pcitem.sentences)->size(); i++){
        Ptr<History> history = histories->at(i);
        Ptr<Request> request = ((pcitem.sentences)->at(i)).request;
        int index = ((pcitem.sentences)->at(i)).index;
        request->set_translation(index, history);
      }
    }
  }
}

}  // namespace bergamot
}  // namespace marian
