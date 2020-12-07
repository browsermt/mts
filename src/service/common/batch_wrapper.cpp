#include "batch_wrapper.h"
namespace marian {
namespace service {

std::vector<std::weak_ptr<Job>>::const_iterator
BatchWrapper::
begin() const {
  return jobs_.begin();
}
std::vector<std::weak_ptr<Job>>::const_iterator
BatchWrapper::
end() const {
  return jobs_.end();
}

std::weak_ptr<Job>
BatchWrapper::
operator[](size_t i) const {
  return jobs_.at(i);
}
} // end of namespace marian::server
} // end of namespace marian
