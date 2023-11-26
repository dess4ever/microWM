#include "log.h"
using json = nlohmann::json;
// nous devons convertir LogSeverity en string pour la sérialisation JSON
std::string logSeverityToString(LogSeverity logSeverity) {
  switch (logSeverity) {
  case LogSeverity::Info: 
    return "Info";
  case LogSeverity::Warning:
    return "Warning";
  case LogSeverity::Error:
    return "Error";
  case LogSeverity::Danger:
    return "Danger";
  case LogSeverity::Detail:
    return "Detail";
  case LogSeverity::fTrace:
    return "fTrace";
  case LogSeverity::sTrace:
    return "sTrace";
  default:
    return "";
  }
}

// nous devons convertir la string en LogSeverity pour la désérialisation
LogSeverity stringToLogSeverity(const std::string& logSeverity) {
  if (logSeverity == "Info") return LogSeverity::Info;
  if (logSeverity == "Warning") return LogSeverity::Warning;
  if (logSeverity == "Error") return LogSeverity::Error;
  if (logSeverity == "Danger") return LogSeverity::Danger;
  if (logSeverity == "Detail") return LogSeverity::Detail;
  if (logSeverity == "fTrace") return LogSeverity::fTrace;
  if (logSeverity == "sTrace") return LogSeverity::sTrace;
  
  throw std::runtime_error("Invalid string value for LogSeverity");
}

std::string serializeLog(const Log *log) {
  json j;
  j["time"] = std::chrono::system_clock::to_time_t(log->time);  // conversion en time_t pour la sérialisation
  j["severity"] = logSeverityToString(log->severity);
  j["fromFunction"] = log->fromFunction;
  j["message"] = log->message;
  return j.dump();
}

Log deserializeLog(const std::string &serializedLog) {
  json j = json::parse(serializedLog);
  Log log;
  log.time = std::chrono::system_clock::from_time_t(j["time"]);  // conversion de time_t en std::chrono::system_clock::time_point
  log.severity = stringToLogSeverity(j["severity"]);
  log.fromFunction = j["fromFunction"];
  log.message = j["message"];
  return log;
}

std::string serializeLogs(const std::vector<Log> &logs) {
  json j = json::array();
  for (const auto &log : logs) {
      j.push_back(json::parse(serializeLog(&log)));
  }
  return j.dump();
}

std::vector<Log> deserializeLogs(const std::string &serializedLogs) {
  std::vector<Log> logs;
  json j = json::parse(serializedLogs);
  for (const auto &log : j) {
      logs.push_back(deserializeLog(log.dump()));
  }
  return logs;
}
