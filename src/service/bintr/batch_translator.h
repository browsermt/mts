// -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
#pragma once

#include <ctime>
#include <functional>
#include <string>
#include <vector>
#include <map>

#include "common/logging.h"
#include "common/utils.h"

#include "data/batch_generator.h"
#include "data/corpus.h"
#include "data/shortlist.h"
#include "data/text_input.h"

#include "translator/history.h"
#include "translator/scorers.h"

// #include "translator/output_collector.h"
// #include "translator/output_printer.h"
// #include "models/model_task.h"

// TODO(jerin): Mark following out, queueing mechanism not in Batching layer.
// #include "queued_input.h"
// #include "queued_input.h"
// #include <thread>

extern Logger logger;

class BatchTranslator {
private:
  DeviceId device_;

  std::function<void(Ptr<History const>)> callback_;
  Ptr<Options> options_;
  Ptr<ExpressionGraph> graph_;
  std::vector<Ptr<Vocab const>> vocabs_;
  std::vector<Ptr<Scorer>> scorers_;
  Ptr<data::ShortlistGenerator const> slgen_;

public:

  BatchTranslator(DeviceId const device,
            std::vector<Ptr<Vocab const>> vocabs,
            Ptr<data::ShortlistGenerator const> slgen,
            Ptr<data::QueuedInput> job_queue,
            std::function<void (Ptr<History const>)> callback,
            Ptr<Options> options);

  template<class BatchType>
  void translate_batch(BatchType batch);
};