// -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
#pragma once

// @TODO: - priority handling of translation requests (for faster premium service)

#include <ctime>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include "ssplit/ssplit.h"
#include "common/types.h" // missing in shortlist.h
#include "common/logging.h"
#include "common/definitions.h"
#include "common/utils.h"
#include "data/shortlist.h"
#include "queued_input.h"
#include "translation_job.h"
#include "translation_worker.h"
#include "translation_options.h"
#include "translator/beam_search.h"
#include "translator/history.h"
#include "service/quality_estimation/qe.h"
#ifdef __CUDA_ARCH__
#include <cuda.h>
#endif

extern Logger logger;

namespace marian {
namespace server {

typedef sentencepiece::SentencePieceText TokenizedSentence;
typedef sentencepiece::min_string_view StringView;

class PlainTextTranslation;
std::vector<Ptr<const Vocab> > loadVocabularies(Ptr<Options> options);

class TranslationService {
  std::mutex lock_; // for management of jobs in progress
  typedef std::pair<Ptr<Job>, std::promise<Ptr<Job const>>> JobEntry;
  std::unordered_map<uint64_t, JobEntry> scheduled_jobs_;

  Ptr<Options> options_; // options set at start of service
  Ptr<qe::QualityEstimator> qualityEstimator_; // Quality estimation module
  TranslationOptions dflt_topts_; // can be set by client

  std::vector<Ptr<Vocab const>> vocabs_;
  Ptr<data::QueuedInput> jq_;
  Ptr<data::ShortlistGenerator const> slgen_;
  ug::ssplit::SentenceSplitter ssplit_;
  std::vector<Ptr<TranslationWorker>> workers_;
  bool keep_going_{true};

  void callback_(Ptr<const History> h);
  void chooseDevice_(Ptr<Options> options);

public:
  typedef ug::ssplit::SentenceStream::splitmode ssplitmode;
  typedef std::function<void(uint64_t ejid,Ptr<History const>h)>ResponseHandler;

  TranslationService(Ptr<Options> options);
  ~TranslationService();

  template<typename Search>
  void start() {
    keep_going_ = true;
    vocabs_ = loadVocabularies(options_);
    if(options_->hasAndNotEmpty("shortlist")) {
      Ptr<data::ShortlistGenerator const> slgen;
      int srcIdx=0, trgIdx=1;
      bool shared_vcb = vocabs_.front() == vocabs_.back();
      slgen_ = New<data::LexicalShortlistGenerator> \
        (options_, vocabs_.front(), vocabs_.back(), srcIdx, trgIdx, shared_vcb);
    }
    jq_.reset(new data::QueuedInput(vocabs_, options_));
    auto devices = Config::getDevices(options_);
    for (auto d: devices) {
      // callback() is not static, so we must wrap it in a lambda:
      auto cb = [=](Ptr<History const> h) { this->callback_(h); };
      auto w = New<TranslationWorker>(d, vocabs_, slgen_, jq_, cb, options_);
      w->template start<Search>();
      workers_.push_back(w);
    }
  }

  std::shared_ptr<const Job>
  push(uint64_t ejid,
       const StringView& input,
       const TranslationOptions* topts=NULL,
       uint32_t offset=0, // start position in original string
       const size_t priority=0, // currently has no effect, yet to be implemented
       std::function<void (Job& j)> callback =[=](Job& j){return;});


  // remove Ptr<Job> from the internal cache
  std::shared_ptr<Job>
  detach(uint64_t ijid); // ijid = internal job id

  void stop();
  bool isRight2LeftDecoder() const;
  Ptr<const Vocab> vocab(int i) const;

  ug::ssplit::SentenceStream
  createSentenceStream(std::string const& input, ssplitmode const& mode);

  Ptr<PlainTextTranslation>
  translate(std::string const& input,
            ssplitmode const smode = ssplitmode::wrapped_text);

  const qe::QualityEstimator&
  QE() const;

};

ug::ssplit::SentenceStream::splitmode
string2splitmode(const std::string& m, bool throwOnError=false);

SentencePieceText tokenize(StringView text, const Vocab& V);

}} // end of namespace marian::server
