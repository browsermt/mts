#ifndef __BERGAMOT_SANELOGGING_H
#define __BERGAMOT_SANELOGGING_H

#include <iostream>
#include "spdlog/spdlog.h"

namespace marian {

#define PLOG(worker, level, ...) checkedPLog(worker, #level, __VA_ARGS__)


template <class... Args>
void checkedPLog(std::string logger, std::string level, Args... args){
  Logger log = spdlog::get(logger);
  if (!log){
    try {
      log = spdlog::daily_logger_st(logger, "logs/"+logger+".log");
    } catch (const spdlog::spdlog_ex& ex) {
      std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
  }

  if(level == "trace")
    log->trace(args...);
  else if(level == "debug")
    log->debug(args...);
  else if(level == "info")
    log->info(args...);
  else if(level == "warn")
    log->warn(args...);
  else if(level == "error")
    log->error(args...);
  else if(level == "critical")
    log->critical(args...);
  else {
    log->warn("Unknown log level '{}' for logger '{}'", level, logger);
  }
  log->flush();

}


}  // namespace marian


#endif // __BERGAMOT_SANELOGGING_H
