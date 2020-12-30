#ifndef __BERGAMOT_MULTIFACTOR_PRIORITY_H
#define __BERGAMOT_MULTIFACTOR_PRIORITY_H

#include "sys/time.h"
#include "data/types.h"
#include "definitions.h"

namespace marian {
  namespace bergamot {

    struct MultiFactorPriority {
      int nice; /* user configurable priority, at a request */
      timeval created; 
      /* What else should priority depend on? */
      double priority(){
        return 1.0;
      }
    };
  }
}

#endif // __BERGAMOT_MULTIFACTOR_PRIORITY_H
