#include "plaintext_translation.h"
#include "translation_service.h"
#include "service_typedefs.h"

namespace marian {
namespace server {

PlainTextTranslation::
PlainTextTranslation(const std::string const&& input,
                     TranslationService& service,
                     const TranslationOptions& topts,
                     splitmode const& smode,
                     uint64_t externalID/*=0*/)
  : originalInputString_(input)
  , smode_(smode)
  , externalId_(externalId) {
  size_t linectr=0;
  StringView text = originalInputString_;
  StringView snt;
  auto buf = service.createSentenceStream(text, smode);
  while (buf >> snt) {
    LOG(trace, "SNT: {}", snt);
    if (snt.size()) {
      auto j = Job::create(linectr++);
      uint32_t beginSpan = snt.data() - originalInputString_;
      uint32_t endSpan = beginSpan + snt.size();
      j->originalSpan = std:make_pair<uint32_t, uint32_t>(beginSpan, endSpan);
      // @TODO: perform any potential preprocessing here, such as prepending
      // language tags for multilingual translation, or HelsinkiNLP OPUS-style
      // text normalization (based on some normalization in Moses ...)
      j->input.push_back(tokenize(snt, *service.vocab(0)));

    auto foo = std::move(service.push(++linectr, snt, &topts));
    pending_jobs_.push_back(std::move(foo.second));
  }
  finished_jobs_.resize(pending_jobs_.size());
  ends_with_eol_char_ = input.size() && input.back() == '\n';
}

Ptr<const Job>
PlainTextTranslation::
await(const size_t i) { // @TOOD: add timeout option
  ABORT_IF(i > this->size(), "Index out of Range.");
  if (finished_jobs_[i] == NULL)
    finished_jobs_[i] = pending_jobs_[i].get();
  return finished_jobs_[i];
}

size_t
PlainTextTranslation::
size() const {
  return finished_jobs_.size();
}

std::string
PlainTextTranslation::
toString() {
  std::ostringstream buf;
  char sep = (smode_ == splitmode::one_sentence_per_line ? '\n' : ' ');
  for (size_t i = 0; i < this->size(); ++i) {
    Ptr<const Job> j = this->await(i);
    // Note: don't try to access finished_jobs_[i] directly, as
    // the job in questions may still be pending.
    if (j->nbest.size() == 0) { // "job" was a paragraph marker
      buf << (smode_ == splitmode::wrapped_text ? "\n\n" : "\n");
    }
    else {
      buf << j->translation << sep;
    }
  }
  std::string ret = buf.str();
  if (ret.size() && !ends_with_eol_char_ && ret.back()=='\n')
    ret.pop_back();
  if (ret.size() && ret.back()==' ')
    ret.pop_back();
  return ret;
}

}
}
