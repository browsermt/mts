#ifndef SRC_BERGAMOT_SERVICE_H_
#define SRC_BERGAMOT_SERVICE_H_

#include "batch_translator.h"
#include "batcher.h"
#include "pcqueue.h"
#include "textops.h"
#include "translation_result.h"

#include <vector>

#include "data/types.h"

namespace marian {
namespace bergamot {

class Service {
public:
  explicit Service(Ptr<Options>);
  std::future<TranslationResult> queue(const string_view &input);
  std::future<TranslationResult> translate(const string_view &input);
  void stop();
  ~Service();

private:
  std::vector<Ptr<Vocab const>> vocabs_;
  TextProcessor text_processor_;
  Batcher batcher_;
  UPtr<PCQueue<PCItem>> pcqueue_;
  std::vector<UPtr<BatchTranslator>> workers_;
  bool running_;
  unsigned int requestId_;
  unsigned int batchNumber_;
};

} // namespace bergamot
} // namespace marian

#endif // SRC_BERGAMOT_SERVICE_H_
