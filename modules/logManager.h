#pragma once
#include "log.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <sstream>

class LogManager
{
private:
    std::string fileName;
    std::string directoryName;
    bool dynamicSave{false};
    bool dynamicPrint{true};
    std::vector<Log> logs;
    int cursor;
    std::ofstream fileStream;
    void printLog(const Log& log);

public:
    LogManager(bool isDynamicPrint);
    ~LogManager();
    // Ajoute le log à la class et si dynamicSave alors ajouter le log dans le fichier, si dynamicPrint afficher le log dans la console en format severity: message avec severity en rouge si danger, en orange si error, en jaune si warning  en vert si info
    void addLog(Log log);
    // Sauvegarde de tous les logs dans un fichier
    void manualSaveAll(const std::string fileName, const std::string pathName);
    // Active l'enregistrement automatique des logs dans le fichier
    void dynamicSaveF(const std::string fileName, const std::string pathName);
    // Stop l'enregistrement automatique des logs et ferme le fichier
    void stopDynamicSave();
    // Ouvre un fichier et ajoute les logs dans logs
    void addLogsFromFiles(const std::string fileName, const std::string pathName);
    // Afficher tous les logs dans la console en format severity: message avec severity en rouge si danger, en orange si error, en jaune si warning  en vert si info
    void printLogBySeverity(LogSeverity severity);
    // afficher le log dans la console en format severity: message avec severity en rouge si danger, en orange si error, en jaune si warning  en vert si info
    void printAllLog();
    // Transforme les logs en json
    std::string getAllLogsForSocket();
    
};

