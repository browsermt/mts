#ifndef __SERVICE_H
#define __SERVICE_H

#include "data/types.h"
#include "translation_result.h"
#include "textops.h"
#include "batch_translator.h"
#include "batcher.h"
#include "multifactor_priority.h"
#include "pcqueue.h"

namespace marian {
namespace bergamot {


class Service {
  TextProcessor text_processor_;
  Batcher batcher_;
  Ptr<std::vector<Ptr<BatchTranslator>>> workers_;
  Ptr<PCQueue<PCItem>> pcqueue_;

 public:
  explicit Service(Ptr<Options>);
  std::future<TranslationResult> trivial_translate(const string_view &);
  std::future<TranslationResult> queue(const string_view &input);
  std::future<TranslationResult> translate(const string_view &input);

  TranslationResult process(Ptr<Segments>, Histories);
  std::string decode(Ptr<History> history);
};

}  // namespace bergamot
}  // namespace marian

#endif // __SERVICE_H
