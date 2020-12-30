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
#include "request.h"

extern Logger logger;

namespace marian {
namespace bergamot {

class BatchTranslator {
public:
  BatchTranslator(const BatchTranslator &) = default;
  BatchTranslator(DeviceId const device, 
                  std::vector<Ptr<Vocab const>> vocabs,
                  Ptr<PCQueue<PCItem>> pcqueue,
                  Ptr<Options> options);

  void translate(const Ptr<Segments>, Ptr<Histories>);
  void mainloop();


private:
  Ptr<Options> options_;

  DeviceId device_;
  std::vector<Ptr<Vocab const>> vocabs_;
  Ptr<ExpressionGraph> graph_;
  std::vector<Ptr<Scorer>> scorers_;
  Ptr<data::ShortlistGenerator const> slgen_;

  Ptr<PCQueue<PCItem>> pcqueue_;
  std::unique_ptr<std::thread> thread_;

};
}  // namespace bergamot
}  // namespace marian

#endif //  __BERGAMOT_BATCH_TRANSLATOR_H
