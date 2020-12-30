#ifndef __BERGAMOT_DEFINITIONS_H
#define __BERGAMOT_DEFINITIONS_H

#include "data/vocab_base.h"
#include <vector>
#include "data/types.h"

namespace marian {
namespace bergamot {

typedef marian::Words Segment;
typedef std::vector<Segment> Segments;
typedef std::vector<marian::string_view> SourceAlignment;
typedef std::vector<SourceAlignment> SourceAlignments;

}  // namespace marian
}  // namespace bergamot

#endif // __BERGAMOT_DEFINITIONS_H
