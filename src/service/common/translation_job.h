// -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
#pragma once
#include <sys/time.h>
#include <thread>
#include "data/vocab.h"
#include "translation_options.h"
#include "translator/history.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include "sentencepiece/src/sentencepiece_processor.h"
#include "sentencepiece.pb.h"
#pragma GCC diagnostic push

namespace marian {
namespace server {

class Error {
  std::string errmsg_;
public:
  Error(std::string const& msg) : errmsg_(msg) {}
  std::string const& str() { return errmsg_; }
};


class TranslationHypothesis {
  // text_:  Text of the translation, with tokenization
  // hardAlignment_: maps from token positions in translation to token positions
  //                 in the (preprocessed) source
  // softAlignment_: distribution of alignment probabilities over source
  // wordScores_: word-level scores
  // hypothesis_: pointer to the hypothesis object as returned from Marian search

  SentencePieceText text_;
  IPtr<const Hypothesis> hypothesis_; // Raw hypothesis as returned by search
public:
  TranslationHypothesis(const Hypothesis:PtrType& h, const Vocab& V);
  IPtr<const Hypothesis> getHypothesis() const;
  // const std::vector<uint32_t>& getHardAlignment() const;
  // const std::vector<uint32_t>& getWordScores() const;
};

class Job : public std::enable_shared_from_this<Job> {
  static std::atomic_ullong job_ctr_;

  Job(uint64_t ejid);

public:
  template<typename ... T>
  static std::shared_ptr<Job> create(T&& ... t) {
    return std::make_shared<Job>(new Job(std::forward<T>(t)...));
  }

  typedef std::pair<struct timeval, struct timezone> timestamp;
  typedef Result nbestlist_item; // see hypothesis.h
  uint64_t const unique_id; // internal job id
  uint64_t external_id{0};  // Client's job id
  stdstd::pair<uint32_t,uint32_t> originalSpan;
  // Original span in original text as submitted by the client to the service.

  // PROCESSING STATISTICS
  int priority{0};    // Job priority; currently has no effect
  timestamp created;  // time this item was created
  timestamp queued;   // time this item entered the queue
  timestamp started;  // time this item left the queue
  timestamp finished; // time this item was translated and postprocessed

  // Shared pointer keesp the batch alive as long as it contains at least
  // one valid job.
  std::shared_ptr<BatchWrapper> batch; // which batch do I belong to?
  std::weak_ptr<PlainTextTranslation> request; // which request do I belong to?
  uint32_t index; // running number (position) within request;

  std::vector<SentencePieceText> input;
  // /input/ is a vector to accommodate multi-source input, although in
  // practice most scenarios are single-input

  size_t nbestlist_size{1}; // single best translation by default
  std::vector<TranslationHypothesis> nbest;
  SentencePiece translation;
  std::vector<sentencepiece::SentencePieceText> tokenization; // for inputs and output
  NBestList nbest; // see translation/history.h for definition
  Ptr<const History> history;

  Ptr<Error> error;
  std::function<void (Job&)> callback;

  void dequeued(); // record start time
  void finish(Ptr<const History> h, const bool R2L, const Vocab& V);

  // functions for keeping track of workflow
  float totalTime() const;
  float timeBeforeQueue() const;
  float timeInQueue() const;
  float translationTime() const;

};
}}
