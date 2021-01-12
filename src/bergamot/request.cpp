#include "request.h"

#include "definitions.h"
#include "translation_result.h"

#include "common/logging.h"

#include <string>

namespace marian {
namespace bergamot {

Request::Request(unsigned int Id, std::vector<Ptr<Vocab const>> &vocabs,
                 string_view reference, Segments &&segments,
                 SourceAlignments &&sourceAlignments,
                 std::promise<TranslationResult> translationResultPromise)
    : Id_(Id), vocabs_(&vocabs), reference_(reference),
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
  TranslationResult translation_result;
  for (int i = 0; i < segments_.size(); i++) {
    std::string source = vocabs_->front()->decode(getSegment(i));
    translation_result.sources.push_back(source);

    Ptr<History> history = histories_[i];
    NBestList onebest = history->nBest(1);
    Result result = onebest[0]; // Expecting only one result;
    Words words = std::get<0>(result);
    std::string decoded = vocabs_->back()->decode(words);
    translation_result.translations.push_back(decoded);
  }
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
