#include "request.h"
#include "definitions.h"
#include "translation_result.h"

namespace marian {
  namespace bergamot {

Request::Request(std::vector<Segment> &segments,
                 std::promise<TranslationResult> &translation_result_promise)
  : segments(&segments){
    /* Construction should mean item is queued for translation  */
    response_ = &translation_result_promise;
    join();
  }

void Request::queue_segments(){
  /*
  static timezone tz = NULL;
  timeval now, delta;
  gettimeofday(now, tz);
  */
}

void Request::cancel() {
  /* Forwards cancellation of sentences ahead. This is probably some
   * book-keeping for this class? */
}

void Request::join() {
  /* Barrier for another operation to wait until request sentences are ready.
  */

  TranslationResult translation_result;
  translation_result.sources.push_back("Stub Input blame Jerin");
  translation_result.translations.push_back("Stub Translation blame Jerin");
  response_->set_value(translation_result);
}

}
}
