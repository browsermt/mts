#ifndef __BERGAMOT_BATCH_TRANSLATOR_H
#define __BERGAMOT_BATCH_TRANSLATOR_H

#include <atomic>
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
#include "definitions.h"
#include "pcqueue.h"
#include "request.h"
#include "translator/beam_search.h"
#include "translator/history.h"
#include "translator/scorers.h"

#include "sanelogging.h"

namespace marian {
namespace bergamot {

class BatchTranslator {
public:
  BatchTranslator(const BatchTranslator &) = default;
  BatchTranslator(DeviceId const device, PCQueue<PCItem> &pcqueue,
                  Ptr<Options> options);

  void initGraph();
  void translate(RequestSentences &, Histories &);
  void mainloop();
  std::string _identifier() { return "worker" + std::to_string(device_.no); }
  void join();

private:
  Ptr<Options> options_;
  DeviceId device_;
  std::vector<Ptr<Vocab const>> vocabs_;
  Ptr<ExpressionGraph> graph_;
  std::vector<Ptr<Scorer>> scorers_;
  Ptr<data::ShortlistGenerator const> slgen_;
  std::atomic<bool> running_{true};
  std::unique_ptr<std::thread> thread_;
  PCQueue<PCItem> *pcqueue_;
};
} // namespace bergamot
} // namespace marian

#endif //  __BERGAMOT_BATCH_TRANSLATOR_H
