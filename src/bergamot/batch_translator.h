#ifndef __BERGAMOT_BATCH_TRANSLATOR_H
#define __BERGAMOT_BATCH_TRANSLATOR_H

#include <ctime>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "common/logging.h"
#include "common/utils.h"
#include "data/batch_generator.h"
#include "data/corpus.h"
#include "data/shortlist.h"
#include "data/text_input.h"
#include "translator/history.h"
#include "translator/scorers.h"
#include "definitions.h"

extern Logger logger;

namespace marian {
namespace bergamot {
class BatchTranslator {
 private:
  DeviceId device_;

  std::function<void(Ptr<History const>)> callback_;
  Ptr<Options> options_;
  Ptr<ExpressionGraph> graph_;
  std::vector<Ptr<Vocab const>> vocabs_;
  std::vector<Ptr<Scorer>> scorers_;
  Ptr<data::ShortlistGenerator const> slgen_;

 public:
  BatchTranslator() { std::cerr << "TODO(jerin): Fix bad constructor\n"; }
  BatchTranslator(const BatchTranslator &) = default;
  BatchTranslator(DeviceId const device, std::vector<Ptr<Vocab const>> vocabs,
                  /* std::function<void(Ptr<History const>)> callback,*/
                  Ptr<Options> options);

  template <class BatchType, class Search>
  Histories translate_batch(BatchType batch) {
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
    auto search = New<Search>(options_, scorers_, trgVocab);

    // The below repeated for a batch?
    auto histories = search->search(graph_, batch);
    return histories;
  }
  marian::Ptr<data::CorpusBatch> construct_batch(
      const std::vector<data::SentenceTuple> &);
  marian::Ptr<data::CorpusBatch> construct_batch_from_segments(
      const Ptr<Segments>);
};
}  // namespace bergamot
}  // namespace marian

#endif //  __BERGAMOT_BATCH_TRANSLATOR_H
