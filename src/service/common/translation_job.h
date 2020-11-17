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


class Job {
  static std::atomic_ullong job_ctr_;


public:
  typedef std::pair<struct timeval, struct timezone> timestamp;
  typedef Result nbestlist_item; // see hypothesis.h
  uint64_t const unique_id; // internal job id
  uint64_t external_id{0}; // Client's job id
  int         priority{0}; // Job priority; currently not used
  timestamp    created; // time item was created
  timestamp     queued; // time item entered the queue
  timestamp    started; // time item left the queue
  timestamp   finished; // time item was translated and postprocessed
  const std::vector<std::string> input;
  const size_t nbestlist_size{1};
  std::string translation;
  std::vector<sentencepiece::SentencePieceText> tokenization; // for inputs and output
  NBestList nbest; // see translation/history.h for definition
  Ptr<const History> history;

  Ptr<Error> error;
  std::function<void (Ptr<Job>)> callback;

  Job(uint64_t ejid, const std::string text,
      const TranslationOptions& topts, const size_t pri=0);

  void dequeued(); // record start time
  void finish(Ptr<const History> h, const bool R2L, const Vocab& V);

  // functions for keeping track of workflow
  float totalTime() const;
  float timeBeforeQueue() const;
  float timeInQueue() const;
  float translationTime() const;

};
}}
