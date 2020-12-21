#pragma once
#include "translator/beam_search.h"
#include "data/types.h"
#include "translation_result.h"
#include "textops.h"
#include "batch_translator.h"
#include "batcher.h"
#include "multifactor_priority.h"

namespace marian {
namespace bergamot {

class Service {
  TextProcessor text_processor_;
  BatchTranslator batch_translator_;
  Batcher batcher_;

 public:
  Service(Ptr<Options> );
  std::future<TranslationResult> trivial_translate(string_view &) ;
  std::future<TranslationResult> queue(string_view &input);
  std::future<TranslationResult> translate(string_view &input);
};

}  // namespace bergamot
}  // namespace marian
