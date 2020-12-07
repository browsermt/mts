#include "sys/time.h"

struct MultiFactorPriority {
  /* Some form of priority. Replace with this on a PriorityQueue */
  unsigned int index_;
  timeval created_;
  static timezone tz = NULL;
  MultiFactorPriority(int index, timeval& created)
      : index_(index), created_(created) {}

  float evaluate(const timeval& baseline) {
    /* Getting time spent since created. */
    timeval now, delta;
    gettimeofday(now, tz);
    timeval_subtract_(delta, now, created);

    /* Time aspect is ignored at the moment
     * TODO(jerin): Extend
     */

    return index;
  }
}

/* Trick the priority queue */
friend bool
operator<(MultiFactorPriority& a, MultiFactorPriority& b) {
  return a.evaluate() < b.evaluate();
}
