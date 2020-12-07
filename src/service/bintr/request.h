#include "data/types.h"

class TranslationResult;

struct Request{
  /* Wraps around a request, to execute barrier/synchronization */

  std::Promise<TranslationResult> response;

  Request(int index, std::vector<Words> &segments){
    /* Construction should mean item is queued for translation  */

  }

  void cancel(){
    /* Forwards cancellation of sentences ahead. This is probably some book-keeping for this class? */
  }

  void join(){
    /* Barrier for another operation to wait until request sentences are ready. */
  }

};