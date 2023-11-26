#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip> 
#include <vector>
#include <nlohmann/json.hpp>

enum LogSeverity { Info, Warning, Error, Danger, Detail, fTrace, sTrace };

struct Log {
    std::chrono::system_clock::time_point time;
    LogSeverity severity{LogSeverity::Info};
    std::string fromFunction;
    std::string message;
    Log(const std::string& m, LogSeverity s, const std::string& c)
      : message(m), severity(s), fromFunction(c) {}
    Log(){}
};

std::string logSeverityToString(LogSeverity logSeverity);
LogSeverity stringToLogSeverity(const std::string& logSeverity);
std::string serializeLog(const Log *log);
Log deserializeLog(const std::string &serializedLog);
std::string serializeLogs(const std::vector<Log> &logs);
std::vector<Log> deserializeLogs(const std::string &serializedLogs);
