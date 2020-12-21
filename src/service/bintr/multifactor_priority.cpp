#include "multifactor_priority.h"

namespace marian {
  namespace bergamot {

    MultiFactorPriority::MultiFactorPriority(int index, 
        timeval& created, 
        Request &request)
      : index(index), request(&request) {}

    int MultiFactorPriority::num_tokens(){
      return request->segments[index].size();
    }

    bool operator<(const MultiFactorPriority& a, const MultiFactorPriority& b) {
      /*
         timeval_subtract_(delta, now, created);
         */
      return a.index < b.index;
    };
  }
}
