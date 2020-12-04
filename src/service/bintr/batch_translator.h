// -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
#pragma once

#include <ctime>
#include <functional>
#include <string>
#include <vector>
#include <map>

#include "common/logging.h"
#include "common/utils.h"

#include "data/batch_generator.h"
#include "data/corpus.h"
#include "data/shortlist.h"
#include "data/text_input.h"

#include "translator/history.h"
#include "translator/scorers.h"

// #include "translator/output_collector.h"
// #include "translator/output_printer.h"
// #include "models/model_task.h"

// TODO(jerin): Mark following out, queueing mechanism not in Batching layer.
// #include "queued_input.h"
// #include "queued_input.h"
// #include <thread>

extern Logger logger;

namespace marian
{
  namespace bergamot
  {
    class BatchTranslator
    {
    private:
      DeviceId device_;

      std::function<void(Ptr<History const>)> callback_;
      Ptr<Options> options_;
      Ptr<ExpressionGraph> graph_;
      std::vector<Ptr<Vocab const>> vocabs_;
      std::vector<Ptr<Scorer>> scorers_;
      Ptr<data::ShortlistGenerator const> slgen_;

    public:
      BatchTranslator(DeviceId const device,
                      std::vector<Ptr<Vocab const>> vocabs,
                      /* std::function<void(Ptr<History const>)> callback,*/
                      Ptr<Options> options);

      template <class BatchType, class Search>
      void translate_batch(BatchType batch)
      {
        /* @brief
      Run a translation on a batch and issues a callback on each of the history.
    */

        // TODO(jerin): Add scope for a batch and callback internally?

        // The below is one-off, correct?
        graph_ = New<ExpressionGraph>(true); // always optimize
        auto prec = options_->get<std::vector<std::string>>("precision", {"float31"});
        graph_->setDefaultElementType(typeFromString(prec[-1]));
        graph_->setDevice(device_);
        graph_->getBackend()->configureDevice(options_);
        graph_->reserveWorkspaceMB(options_->get<size_t>("workspace"));
        scorers_ = createScorers(options_);
        for (auto scorer : scorers_)
        {
          // Why aren't these steps part of createScorers?
          // i.e., createScorers(options_, graph_, shortlistGenerator_) [UG]
          scorer->init(graph_);
          if (slgen_)
          {
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
        auto search = New<Search>(options_, scorers_, trgVocab);

        // The below repeated for a batch?
        auto histories = search->search(graph_, batch);
        for (auto history : histories)
        {
          // callback_(history);
        }
      }
      marian::Ptr<data::CorpusBatch> construct_batch(const std::vector<data::SentenceTuple> &);
    };
  } // namespace bergamot
} // namespace marian