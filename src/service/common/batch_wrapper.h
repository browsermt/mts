#pragma once
#include "marian.h"
#include <memory>

// Wrapper around the standard marian batch, with an additional
// vector of jobs covered by this batch.
// The purpose of this class is to keep track of the marian::server::Jobs
// that the batch consists of, so that we can call the setResult(...) function
// on each job in the batch.

namespace marian {
namespace server {
class Job;
class BatchWrapper : public marian::Batch {
  std::vector<std::weak_ptr<Job>> jobs_;
  // Jobs are owned by whover pushed them to the service.
  // Jobs keep a shared ptr to the BatchWrapper that contains them.
  // If all jobs in a batch go away, the batch becomes invalid.

public:
  std::vector<std::weak_ptr<Job>>::const_iterator begin() const;
  std::vector<std::weak_ptr<Job>>::const_iterator end() const;
  std::weak_ptr<Job> operator[](size_t i) const;
}
