#pragma once
#include "sys/time.h"
#include "request.h"

namespace marian {
  namespace bergamot {

    class Request;
    struct MultiFactorPriority {
      /* Some form of priority. Replace with this on a PriorityQueue */
        unsigned int index;
        Request *request;
        MultiFactorPriority(int, timeval& , Request &);
        int num_tokens();
    };

    bool operator<(const MultiFactorPriority& a, const MultiFactorPriority& b);

  }
}

