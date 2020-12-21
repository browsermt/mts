#include "request.h"
#include "definitions.h"
#include "translation_result.h"
#include "sys/time.h"
#include <iostream>

namespace marian {
  namespace bergamot {
    Request::Request(Ptr<std::vector<Segment>> segments,
        std::promise<TranslationResult> &translation_result_promise)
      : segments(segments){
        /* Construction should mean item is queued for translation  */
        struct timezone *tz = NULL;
        gettimeofday(&created, tz);
        response_ = &translation_result_promise;
        join();
      }

    void Request::cancel() {
      /* Forwards cancellation of sentences ahead. This is probably some
       * book-keeping for this class? */
    }

    void Request::set_translation(int index, Ptr<History> history){
    
    }

    void Request::join() {
      /* Barrier for another operation to wait until request sentences are ready.
      */

      TranslationResult translation_result;
      translation_result.sources.push_back("Stub Input blame Jerin");
      translation_result.translations.push_back("Stub Translation blame Jerin");
      response_->set_value(translation_result);
    }

    bool operator<(const Request &a, const Request &b){
      return a.created.tv_sec < b.created.tv_sec;
    }

  }
}
