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
#include "translator/beam_search.h"
#include "pcqueue.h"
#include "multifactor_priority.h"

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
  Ptr<PCQueue<PCItem>> pcqueue_;
  std::unique_ptr<std::thread> thread_;

 public:
  BatchTranslator(const BatchTranslator &) = default;
  BatchTranslator(DeviceId const device, 
                  std::vector<Ptr<Vocab const>> vocabs,
                  /* std::function<void(Ptr<History const>)> callback,*/
                  Ptr<PCQueue<PCItem>> pcqueue,
                  Ptr<Options> options);

  Histories translate_batch(Ptr<data::CorpusBatch> batch);
  marian::Ptr<data::CorpusBatch> construct_batch(
      const std::vector<data::SentenceTuple> &);
  marian::Ptr<data::CorpusBatch> construct_batch_from_segments(
      const Ptr<Segments>);
  std::string decode(Ptr<History>);

  Histories translate_segments(Ptr<Segments>);

  void mainloop(){
    while(true){
      PCItem pcitem;
      pcqueue_->Consume(pcitem);
      auto histories = translate_segments(pcitem.segments);
      for(int i=0; i < (pcitem.sentences)->size(); i++){
        Ptr<History> history = histories[i];
        Ptr<Request> request = ((pcitem.sentences)->at(i)).request;
        int index = ((pcitem.sentences)->at(i)).index;
        request->set_translation(index, decode(history));
      }
    }
  }

};
}  // namespace bergamot
}  // namespace marian

#endif //  __BERGAMOT_BATCH_TRANSLATOR_H
