#pragma once
#include <future>
#include <vector>
#include <string>
#include <map>
#include "common/definitions.h"
#include "service_typedefs.h"
#include "ssplit/ssplit.h"
#include "translation_options.h"
namespace marian {
namespace server {

class TranslationService;
class Job;

class PlainTextTranslation
  : public std::enable_shared_from_this<PlainTextTranslation> {
public:
  typedef  ug::ssplit::SentenceStream::splitmode splitmode;

  // factory function
  template<typename ... T>
  static std::shared_ptr<PlainTextTranslation> create(T&& ... t) {
    return std::make_shared<PlainTextTranslation>(new PlainTextTranslation(std::forward<T>(t)...));
  }

private:
  const std::string originalInputString_;
  // originalInputString_ may or may not contain the original input
  // string, depending on the constructor called. The variant with
  // the const std::string&& takes ownership of the original input.

  // relevant for sentence splitting/joining
  splitmode smode_; // sentence splitting mode
  bool ends_with_eol_char_{false};

  std::vector<std::future<Ptr<Job const>>> pending_jobs_;
  std::vector<Ptr<Job const>> finished_jobs_;

  // book-keeping and multithreading
  uint64_t externalId_; // external job id
  std::mutex mutex_; // mutex for controlling access
  std::condition_variable ready_; // for signalling that translation has finished
  static std::atomic_ullong pending_; // counter of pending jobs

  PlainTextTranslation(std::string const&& input,
                       TranslationService& service,
                       const TranslationOptions& topts,
                       splitmode const& smode,
                       uint64_t ejid=0);

public:
  // @TODO: implement a custom iterator that allows iteration
  // over partially finished PlainTextTranslation, to accommodate
  // translating large amounts of text. We can't just use an
  // iterator over finished_jobs_, because jobs may not have finished
  // by the time someone tries to access the results.


  void notify(size_t i); // notify that job #i is done

  Ptr<const Job> await(const size_t i);

  size_t size() const;

  // void await() {
  //   for (size_t i = 0; i < finished_jobs.size(); ++i) {
  //     if (finished_jobs_[i] == NULL) {
  //       finished_jobs_[i] = pending_jobs_[i].get();
  //     }
  //   }
  // }

  std::string toString();
};
}} // end of namespace marian::server
