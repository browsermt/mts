#include "common/logging.h"
#include "output_options.h"
#include "rapidjson_utils.h"
namespace marian {
  namespace server {

    bool OutputOptions::noDetails() const {
      // we don't need to check withWordAlignment and withSoftAlignment, because
      // these imply withTokenization
      return !(withOriginal ||
          withQualityEstimate ||
          withSentenceScore ||
          withSoftAlignment ||
          withTokenization ||
          withWordAlignment ||
          withWordScores);
    }

  }}// end of namespace marian::server
