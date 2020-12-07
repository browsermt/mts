#pragma once
#include "service/common/queue.h"
#include "service/common/translation_job.h"
#include <stdint.h>
#include <vector>
#include <atomic>
#include <sys/time.h>

namespace marian {
namespace server {

class BatchQueue {
