#include "window.h"

using json = nlohmann::json;

// Convertit un EWMHtype enum en string
std::string ewmhTypeToString(EWMHtype type) {
    switch (type) {
        case EWMH_WINDOW_TYPE_DESKTOP: return "DESKTOP";
        case EWMH_WINDOW_TYPE_DOCK: return "DOCK";
        case EWMH_WINDOW_TYPE_TOOLBAR: return "TOOLBAR";
        case EWMH_WINDOW_TYPE_MENU: return "MENU";
        case EWMH_WINDOW_TYPE_UTILITY: return "UTILITY";
        case EWMH_WINDOW_TYPE_SPLASH: return "SPLASH";
        case EWMH_WINDOW_TYPE_DIALOG: return "DIALOG";
        case EWMH_WINDOW_TYPE_NORMAL: return "NORMAL";
        case EWMH_WINDOW_TYPE_DROPDOWN_MENU: return "DROPDOWN_MENU";
        case EWMH_WINDOW_TYPE_POPUP_MENU: return "POPUP_MENU";
        case EWMH_WINDOW_TYPE_TOOLTIP: return "TOOLTIP";
        case EWMH_WINDOW_TYPE_NOTIFICATION: return "NOTIFICATION";
        case EWMH_WINDOW_TYPE_COMBO: return "COMBO";
        case EWMH_WINDOW_TYPE_DND: return "DND";
        default: return "UNKNOWN";
    }
}

// Convertit un string en EWMHtype enum
EWMHtype stringToEwmhType(const std::string& str) {
    if (str == "DESKTOP") return EWMH_WINDOW_TYPE_DESKTOP;
    if (str == "DOCK") return EWMH_WINDOW_TYPE_DOCK;
    if (str == "TOOLBAR") return EWMH_WINDOW_TYPE_TOOLBAR;
    if (str == "MENU") return EWMH_WINDOW_TYPE_MENU;
    if (str == "UTILITY") return EWMH_WINDOW_TYPE_UTILITY;
    if (str == "SPLASH") return EWMH_WINDOW_TYPE_SPLASH;
    if (str == "DIALOG") return EWMH_WINDOW_TYPE_DIALOG;
    if (str == "NORMAL") return EWMH_WINDOW_TYPE_NORMAL;
    if (str == "DROPDOWN_MENU") return EWMH_WINDOW_TYPE_DROPDOWN_MENU;
    if (str == "POPUP_MENU") return EWMH_WINDOW_TYPE_POPUP_MENU;
    if (str == "TOOLTIP") return EWMH_WINDOW_TYPE_TOOLTIP;
    if (str == "NOTIFICATION") return EWMH_WINDOW_TYPE_NOTIFICATION;
    if (str == "COMBO") return EWMH_WINDOW_TYPE_COMBO;
    if (str == "DND") return EWMH_WINDOW_TYPE_DND;
    // Assumez que tout type non reconnu est UNKNOWN
    return EWMH_WINDOW_TYPE_UNKNOWN;
}

// Fonction utilitaire pour sérialiser un vecteur d'EWMHState
json serializeEWMHStates(const std::vector<EWMHSTATES>& atoms) {
    json jStates = json::array();
    for (EWMHSTATES state : atoms) {
        jStates.push_back(static_cast<int>(state)); // Convertit l'enum en int pour le JSON
    }
    return jStates;
}

// Fonction utilitaire pour désérialiser un vecteur d'EWMHState
std::vector<EWMHSTATES> deserializeEWMHSTATES(const json& jAtoms) {
    std::vector<EWMHSTATES> states;
    for (const auto& jState : jAtoms) {
        states.push_back(static_cast<EWMHSTATES>(jState.get<int>()));
    }
    return states;
}

// Fonction utilitaire pour sérialiser WMNormalHints
json serializeWMNormalHints(const WMNormalHints& hints) {
    json j;
    j["flags"] = hints.flags;
    // Ajoutez d'autres champs ici...
    return j;
}

// Fonction utilitaire pour désérialiser WMNormalHints
WMNormalHints deserializeWMNormalHints(const json& j) {
    WMNormalHints hints;
    hints.flags = j["flags"].get<int>();
    // Ajoutez d'autres champs ici...
    return hints;
}

// Sérialise un objet Window en string JSON
std::string serializeWindow(const Window &window) {
    json j;
    j["normalConfiguration"] = json::parse(serializeConfigurationWindow(window.normalConfiguration));
    j["states"] = serializeEWMHStates(window.atoms);
    j["normalHints"] = json::array(); // À remplacer par la sérialisation réelle des normalHints
    j["isOverrideRedirect"] = window.isOverrideRedirect;
    j["transientFor"] = window.transientFor;
    j["type"] = ewmhTypeToString(window.type);
    return j.dump();
}

// Désérialise une string JSON en objet Window
Window deserializeWindow(const std::string &serializedWindow) {
    json j = json::parse(serializedWindow);
    Window window;
    window.normalConfiguration = deserializeConfigurationWindow(j["normalConfiguration"].dump());
    window.atoms = deserializeEWMHSTATES(j["states"]);
    window.isOverrideRedirect = j["isOverrideRedirect"].get<bool>();
    window.transientFor = j["transientFor"].get<int>();
    window.type = stringToEwmhType(j["type"].get<std::string>());
    // Les normalHints doivent également être désérialisés, si nécessaire...
    return window;
}

// Sérialise un vecteur d'objets Window en string JSON
std::string serializeWindows(const std::vector<Window> &windows) {
    json j = json::array();
    for (const Window& window : windows) {
        j.push_back(json::parse(serializeWindow(window)));
    }
    return j.dump();
}

// Désérialise une string JSON en vecteur d'objets Window
std::vector<Window> deserializeWindows(const std::string &serializedWindows) {
    std::vector<Window> windows;
    json j = json::parse(serializedWindows);
    for (const auto& serializedWindow : j) {
        windows.push_back(deserializeWindow(serializedWindow.dump()));
    }
    return windows;
}

// Convertit un objet Window en string descriptive (pour le debug, par exemple)
std::string windowToString(const Window &window) {
    // Ceci est une fonction d'appoint qui ne sérialise pas complètement l'objet
    // mais renvoie une représentation string simplifiée
    std::stringstream ss;
    ss << "Window ID: " << window.normalConfiguration.windowId << ", ";
    ss << "Type: " << ewmhTypeToString(window.type) << ", ";
    ss << "States: ";
    for (EWMHSTATES state : window.atoms) {
        ss << static_cast<int>(state) << " ";
    }
    ss << "(...)";
    return ss.str();
}