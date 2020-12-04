#include "batch_translator.h"

namespace marian
{
  namespace bergamot
  {

    BatchTranslator::BatchTranslator(DeviceId const device,
                                     std::vector<Ptr<Vocab const>> vocabs,
                                     /* std::function<void(Ptr<History const>)> callback,*/
                                     Ptr<Options> options)
        : device_(device), /*callback_(callback),*/
          options_(options), vocabs_(vocabs)
    {
      if (options_->hasAndNotEmpty("shortlist"))
      {
        Ptr<data::ShortlistGenerator const> slgen;
        int srcIdx = 0, trgIdx = 1;
        bool shared_vcb = vocabs_.front() == vocabs_.back();
        slgen_ = New<data::LexicalShortlistGenerator>(options_, vocabs_.front(), vocabs_.back(), srcIdx, trgIdx, shared_vcb);
      }
    }

    Ptr<data::CorpusBatch> BatchTranslator::construct_batch(const std::vector<data::SentenceTuple> &batchVector)
    {

      // TODO(jerin): The following is to fix rest, take off, absolute danger.
      size_t batchSize = batchVector.size();

      std::vector<size_t> sentenceIds;

      std::vector<int> maxDims;
      for (auto &ex : batchVector)
      {
        if (maxDims.size() < ex.size())
          maxDims.resize(ex.size(), 0);
        for (size_t i = 0; i < ex.size(); ++i)
        {
          if (ex[i].size() > (size_t)maxDims[i])
            maxDims[i] = (int)ex[i].size();
        }
        sentenceIds.push_back(ex.getId());
      }

      typedef marian::data::SubBatch SubBatch;
      typedef marian::data::CorpusBatch CorpusBatch;

      std::vector<Ptr<SubBatch>> subBatches;
      for (size_t j = 0; j < maxDims.size(); ++j)
      {
        subBatches.emplace_back(New<SubBatch>(batchSize, maxDims[j], vocabs_[j]));
      }

      std::vector<size_t> words(maxDims.size(), 0);
      for (size_t i = 0; i < batchSize; ++i)
      {
        for (size_t j = 0; j < maxDims.size(); ++j)
        {
          for (size_t k = 0; k < batchVector[i][j].size(); ++k)
          {
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
      return batch;
    }

  } // namespace bergamot
} // namespace marian