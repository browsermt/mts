#include "sys/time.h"


struct MultiFactorPriority {
  /* Some form of priority. Replace with this on a PriorityQueue */
  unsigned int index;
  timeval created;
  static timezone tz = NULL;

  float evaluate(const timeval &baseline){
    /* Return a computed priority value. */

    /* Getting time spent since created. */
    timeval now, delta;
    gettimeofday(now, tz);
    timeval_subtract_(delta, now, created);

    /* Time aspect is ignored at the moment 
     * TODO(jerin): Extend
     * /
    return index;
  }
}


/* Trick the priority queue */
friend bool operator< (MultiFactorPriority& a, MultiFactorPriority& b){
  return a.evaluate() < b.evaluate();
}

