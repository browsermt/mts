#include "multifactor_priority.h"

namespace marian {
  namespace bergamot {

    MultiFactorPriority::MultiFactorPriority(int index, 
        Ptr<Request> request)
      : index(index), request(request) {}

    int MultiFactorPriority::num_tokens(){
      return (request->segments->at(index).size());
    }

    bool operator<(const MultiFactorPriority& a, const MultiFactorPriority& b) {
      if(a.request == b.request){
        return a.index < b.index;
      }
      return a < b;
    };

    bool operator==(const MultiFactorPriority& a, const MultiFactorPriority& b) {
      return (a.index == b.index) and (a.request == b.request);
    };

  }
}
