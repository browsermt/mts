#ifndef __BERGAMOT_MULTIFACTOR_PRIORITY_H
#define __BERGAMOT_MULTIFACTOR_PRIORITY_H

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

    struct PCItem {
      Ptr<Segments> segments;
      Ptr<std::vector<MultiFactorPriority>> sentences;
      PCItem(): segments(NULL), sentences(NULL) {}
      PCItem(Ptr<Segments> segments, Ptr<std::vector<MultiFactorPriority>> sentences):
          segments(segments), sentences(sentences){}

      void operator=(const PCItem &b){
        segments = b.segments;
        sentences = b.sentences;
      }
    };

  }

}

#endif // __BERGAMOT_MULTIFACTOR_PRIORITY_H
