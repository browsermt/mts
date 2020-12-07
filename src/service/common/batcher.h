// -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
#pragma once

#include <functional>
#include <string>
#include <vector>
#include <map>

#include "common/logging.h"
#include "data/batch_generator.h"
#include "translation_job.h"
#include <thread>

extern Logger logger;

namespace marian {
namespace server {


class Batcher
{
public:
  struct BatchEnvelope {
    Ptr<const Batch> batch;     // a standard Marian batch
    std::vector<std::weak_tpr<Job>> jobs; // corresponding array of submitted jobs
  };

  void push(const Ptr<Job>&, const TranslationOptions& topts);
  std::uniqe_ptr<BatchEnvelope> pop();

private:

  std::unique_ptr<std::thread> thread_;
  data::QueuedInput greedy_input_queue_; // job input queue for greedy search
  data::QueuedInput  nbest_input_queue_; // job input queue for nbest search
  Queue<BatchEnvelope>    pending_queue; // queue of batches to be launched

  Ptr<Options> options_;
  bool keep_going_{true};

  template<typename Search>
  void run_() {
    init_();
    LOG(info,"Worker {} is ready.", std::string(device_));
    keep_going_ = true;
    while(keep_going_) { // will be set to false by stop()
      data::BatchGenerator<data::QueuedInput> bgen(job_queue_, options_);
      bgen.prepare();
      auto trgVocab = vocabs_.back();
      for (auto b: bgen) {
        auto search = New<Search>(options_, scorers_, trgVocab);
        auto histories = search->search(graph_, b);
        for (auto h: histories)
          callback_(h);
      }
    }
  }

public:
  TranslationWorker(DeviceId const device,
                    std::vector<Ptr<Vocab const>> vocabs,
                    Ptr<data::ShortlistGenerator const> slgen,
                    Ptr<data::QueuedInput> job_queue,
                    std::function<void (Ptr<History const>)> callback,
                    Ptr<Options> options);

  template<typename Search>
  void start() {
    ABORT_IF(thread_ != NULL, "Don't call start on a running worker!");
    thread_.reset(new std::thread([this]{ this->run_<Search>(); }));
  }

  void stop();
  void join();
}; // end of class TranslationWorker

} // end of namespace marian::server
} // end of namespace marian
