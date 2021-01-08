#ifndef __SERVICE_H
#define __SERVICE_H

#include "data/types.h"
#include "translation_result.h"

#include "textops.h"
#include "batcher.h"
#include "batch_translator.h"
#include "pcqueue.h"

namespace marian {
namespace bergamot {


class Service {
public:
  explicit Service(Ptr<Options>);
  std::future<TranslationResult> queue(const string_view &input);
  std::future<TranslationResult> translate(const string_view &input);
  void stop();

private:
  std::vector<Ptr<Vocab const>> vocabs_;
  TextProcessor text_processor_;
  Batcher batcher_;
  std::vector<Ptr<BatchTranslator>> workers_;
  Ptr<PCQueue<PCItem>> pcqueue_;
};

}  // namespace bergamot
}  // namespace marian

#endif // __SERVICE_H
