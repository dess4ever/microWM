#include "logManager.h"

std::ostream& operator<<(std::ostream& os, const LogSeverity& severity)
{
    switch(severity)
    {
        case Info: os << "Info"; break;
        case Warning: os << "Warning"; break;
        case Error: os << "Error"; break;
        case Danger: os << "Danger"; break;
        case Detail: os << "Detail"; break;
        case fTrace: os << "fTrace"; break;
        case sTrace:os << "sTrace"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Log& log)
{
    auto time_c = std::chrono::system_clock::to_time_t(log.time);
    os << std::put_time(std::localtime(&time_c), "%Y-%m-%d %H:%M:%S") << " ";
    os << log.severity << " ";
    os << log.fromFunction << " ";
    os << log.message;
    return os;
}

std::istream& operator>>(std::istream& is, LogSeverity& severity) {
    std::string severityStr;
    is >> severityStr;
    
    if(severityStr == "Info")
        severity = Info;
    else if(severityStr == "Warning")
        severity = Warning;
    else if(severityStr == "Error")
        severity = Error;
    else if(severityStr == "Danger")
        severity = Danger;
    else if(severityStr == "Detail")
        severity = Detail;
    else if(severityStr=="fTrace")
        severity= fTrace;
    else if(severityStr=="sTrace")
        severity=sTrace;
    else  
        is.setstate(std::ios::failbit);

    return is;
}

std::istream& operator>>(std::istream& is, Log& log) {
    std::string timeStr;
    is >> timeStr;
    log.time = std::chrono::system_clock::from_time_t(std::stoll(timeStr)); 

    is >> log.severity;
    is >> log.fromFunction;
    is >> log.message;
    return is;
}

// Constructeur avec paramètre permettant d'activer l'affichage dynamique
LogManager::LogManager(bool isDynamicPrint) : dynamicPrint(isDynamicPrint), cursor(0) {}

// Destructeur responsable de la fermeture du fichier si ouvert
LogManager::~LogManager() {
    if (fileStream.is_open()) {
        fileStream.close();
    }
}

// Ajouter un log à la liste et gérer l'option d'enregistrement et d'affichage dynamiques
void LogManager::addLog(Log log) {
    log.time=std::chrono::system_clock::now();
    logs.push_back(log);
    if (dynamicSave && fileStream.is_open()) {
        fileStream <<  log << std::endl;
    }
    if (dynamicPrint) {
        printLog(log);
    }
}

void LogManager::manualSaveAll(const std::string fileName, const std::string pathName) {
    std::ofstream outputFile(pathName + fileName, std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier pour l'écriture" << std::endl;
        return;
    }
    for (const auto& log : logs) {
        outputFile << log.message << std::endl;
    }
    outputFile.close();
}

void LogManager::dynamicSaveF(const std::string fileName, const std::string pathName) {
    dynamicSave = true;
    fileStream.open(pathName + fileName, std::ios::app);
    if (!fileStream.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier pour l'écriture" << std::endl;
        dynamicSave = false;
    }
}

void LogManager::stopDynamicSave() {
    dynamicSave = false;
    if (fileStream.is_open()) {
        fileStream.close();
    }
}

void LogManager::addLogsFromFiles(const std::string fileName, const std::string pathName) {
    std::ifstream inputFile(pathName + fileName);
    if (!inputFile.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier pour la lecture" << std::endl;
        return;
    }
    Log log;
    while (inputFile >> log) {
        logs.push_back(log);
    }
    inputFile.close();
}

void LogManager::printLogBySeverity(LogSeverity severity) {
    for (const auto& log : logs) {
        if (log.severity == severity) {
            printLog(log);
        }
    }
}

void LogManager::printAllLog() {
    for (const auto& log : logs) {
        printLog(log);
    }
}

std::string LogManager::getAllLogsForSocket() {
    return serializeLogs(logs);
}

std::string LogManager::getFilteredLogByCall(std::string call)
{
    std::vector<Log>tpLogs = logs;
    auto end_it= std::remove_if(tpLogs.begin(),tpLogs.end(),[&](const Log& log)
    {
        return log.fromFunction!=call;
    });
    tpLogs.erase(end_it,tpLogs.end());
    return serializeLogs(tpLogs);
}

// Fonction auxiliaire pour imprimer un log
void LogManager::printLog(const Log& log) {
    std::stringstream ss;
    auto t = std::chrono::system_clock::to_time_t(log.time);
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d %X") << " - ";
    
    switch (log.severity) {
        case Danger:  ss << "\033[1;31mDanger";  break;
        case Error:   ss << "\033[1;33mError";   break;
        case Warning: ss << "\033[1;35mWarning"; break;
        case Info:    ss << "\033[1;32mInfo";    break;
        case Detail:  ss << "\033[1;33mDétail]"; break;
    }
    ss << "\033[0m: " << log.message;
    std::cout << ss.str() << std::endl;
}