#include "data/types.h"
#include "sys/time.h"
#include "translation_result.h"
#include "definitions.h"

namespace marian {
namespace bergamot {
struct Request {
  /* Wraps around a request, to execute barrier/synchronization */

  int index_;
  std::promise<TranslationResult> *response_;

  Request(int index, std::vector<Segment> &segments,
          std::promise<TranslationResult> &translation_result_promise)
      : index_(index) {
    /* Construction should mean item is queued for translation  */
    response_ = &translation_result_promise;
    timeval created;
    gettimeofday(&created, NULL);
    TranslationResult translation_result;
    translation_result.sources.push_back("Stub Input blame Jerin");
    translation_result.translations.push_back("Stub Translation blame Jerin");
    response_->set_value(translation_result);
  }

  void cancel() {
    /* Forwards cancellation of sentences ahead. This is probably some
     * book-keeping for this class? */
  }

  void join() {
    /* Barrier for another operation to wait until request sentences are ready.
     */
  }
};
}  // namespace bergamot
}  // namespace marian