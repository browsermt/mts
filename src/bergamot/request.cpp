#include "request.h"

#include "definitions.h"
#include "translation_result.h"

#include "common/logging.h"

#include <string>

namespace marian {
namespace bergamot {

Request::Request(unsigned int Id, std::vector<Ptr<Vocab const>> &vocabs,
                 std::string &&source, Segments &&segments,
                 SourceAlignments &&sourceAlignments,
                 std::promise<TranslationResult> translationResultPromise)
    : Id_(Id), vocabs_(&vocabs), source_(std::move(source)),
      segments_(std::move(segments)),
      sourceAlignments_(std::move(sourceAlignments)),
      response_(std::move(translationResultPromise)) {

  counter_ = segments_.size();
  histories_.resize(segments_.size(), nullptr);
}

int Request::numSegments() const { return segments_.size(); }

int Request::segmentTokens(int index) const {
  return (segments_[index].size());
}

Segment Request::getSegment(int index) const { return segments_[index]; }

void Request::processHistory(int index, Ptr<History> history) {
  // Concurrently called by multiple workers as a history from translation is
  // ready. The container storing histories is set with the value obtained.
  histories_[index] = history;

  // In case this is last request in, completeRequest is called, which sets the
  // value of the promise.
  if (--counter_ == 0) {
    completeRequest();
  }
}

void Request::completeRequest() {
  // Request no longer needs to hold the content, can transfer it to
  // TranslationResult.
  TranslationResult translation_result(std::move(source_), std::move(segments_),
                                       std::move(sourceAlignments_),
                                       std::move(histories_), *vocabs_);
  LOG(info, "Last translation in. Closing request;");
  response_.set_value(translation_result);
}

bool Request::operator<(const Request &b) const {
  // Among Requests, only sequence id is used for obtaining priority.
  return Id_ < b.Id_;
}

RequestSentence::RequestSentence(int index, Ptr<Request> request)
    : index_(index), request_(request) {}

int RequestSentence::numTokens() { return (request_->segmentTokens(index_)); }

void RequestSentence::completeSentence(Ptr<History> history) {
  // Relays completeSentence into request's processHistory, using index
  // information.
  request_->processHistory(index_, history);
}

Segment RequestSentence::getUnderlyingSegment() const {
  return request_->getSegment(index_);
}

bool operator<(const RequestSentence &a, const RequestSentence &b) {
  // Operator overload for usage in priority-queue / set.
  if (a.request_ == b.request_) {
    return a.index_ < b.index_;
  }
  return a.request_ < b.request_;
}

} // namespace bergamot
} // namespace marian
