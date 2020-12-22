#pragma once
#include "sys/time.h"
#include "request.h"
#include "data/types.h"
#include "definitions.h"

namespace marian {
  namespace bergamot {

    class Request;
    struct MultiFactorPriority {
      /* Some form of priority. Replace with this on a PriorityQueue */
        int index;
        Ptr<Request> request;
        MultiFactorPriority(int, Ptr<Request>);
        int num_tokens();
        Segment segment() const;
        
    };

    bool operator<(const MultiFactorPriority& a, const MultiFactorPriority& b);
    bool operator==(const MultiFactorPriority& a, const MultiFactorPriority& b);

  }
}

