#ifndef __BERGAMOT_BATCH_TRANSLATOR_H
#define __BERGAMOT_BATCH_TRANSLATOR_H

#include <ctime>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <atomic>

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
#include "queue.h"
#include "request.h"

#include "sanelogging.h"

namespace marian {
namespace bergamot {

class BatchTranslator {
public:
  BatchTranslator(const BatchTranslator &) = default;
  BatchTranslator(DeviceId const device, 
                  std::vector<Ptr<Vocab const>> vocabs,
                  Ptr<Queue<PCItem>> pcqueue,
                  Ptr<Options> options);

  void initGraph();
  void translate(const Ptr<Segments>, Ptr<Histories>);
  void mainloop();
  std::string _identifier() { return "worker" + std::to_string(device_.no); }
  void stop(){ running_ = false; }
  void join(){ 
    PLOG(_identifier(), info, "calling join");
    thread_->join(); 
    PLOG(_identifier(), info, "joined");
    PLOG(_identifier(), info, "calling reset");
    thread_.reset(); 
    PLOG(_identifier(), info, "reset");
  }


private:
  Ptr<Options> options_;
  DeviceId device_;
  std::vector<Ptr<Vocab const>> vocabs_;
  Ptr<ExpressionGraph> graph_;
  std::vector<Ptr<Scorer>> scorers_;
  Ptr<data::ShortlistGenerator const> slgen_;
  std::atomic<bool> running_{true};

  Ptr<Queue<PCItem>> pcqueue_;
  std::unique_ptr<std::thread> thread_;
  timeout_t timeout_;

};
}  // namespace bergamot
}  // namespace marian

#endif //  __BERGAMOT_BATCH_TRANSLATOR_H
