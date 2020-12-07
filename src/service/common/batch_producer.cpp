#include "translation_worker.h"
#include "translator/scorers.h"
namespace marian {
namespace server {

BatchProducer::
BatchProducer(Ptr<data::QueuedInput> job_queue,
              Ptr<server::Queue<Ptr<Batch> > > batch_queue,
              Ptr<TranslationService> service,
              Ptr<Options> options)
  : job_queue_(job_queue), callback_(callback),
    options_(options), vocabs_(vocabs), slgen_(slgen)
{ }

void TranslationWorker::stop() {
  keep_going_ = false;
}

void TranslationWorker::join() {
  thread_->join();
  thread_.reset();
}

}
}
