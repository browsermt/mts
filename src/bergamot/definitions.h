#ifndef __BERGAMOT_DEFINITIONS_H
#define __BERGAMOT_DEFINITIONS_H

#include "data/vocab_base.h"
#include <vector>
#include "data/types.h"

typedef marian::Words Segment;
typedef std::vector<Segment> Segments;
typedef std::vector<marian::string_view> Alignment;
typedef std::vector<Alignment> Alignments;

#endif // __BERGAMOT_DEFINITIONS_H
