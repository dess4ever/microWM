#pragma once
#include <xcb/xcb.h>
#include <unordered_map>
#include "displayInfo.h"
#include <png.h>
#include <optional>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "configurationWindow.h"
#include "logManager.h"
#include <memory>
#include "EWMHManager.h"
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <xcb/xcb_keysyms.h>
#include <list>
#include <chrono>
#include <thread>

using Display=microWM::Display;

/*
    * Classe qui gère la connexion au serveur X
    * et qui permet de récupérer des informations sur les fenêtres
    * et de les manipuler
*/

class XcbManager
{
private:
    xcb_connection_t *connection ;
    xcb_screen_t *screen;
    DisplayInfo *info;
    int getDisplayScreen(const ConfigurationWindow configuration);
    EWMHManager *ewmhManager;
    // Fonction de positionnement d'une fenêtre par ratio afin de limiter le code
    void setWindowOnRatio(xcb_window_t window, double xRatio, double yRatio, double widthRatio, double heightRatio);
    // Fonction de positionnement d'une fenêtre par ratio afin de limiter le code mais qui renvoie un Window
    Window getWindowOnRatio(xcb_window_t window, double xRatio, double yRatio, double widthRatio, double heightRatio);
    // Fonction qui applique la géométrie et l'opacité de l'ensemble des fenêtres windows
    void applyGeometryAndOpacity(std::vector<Window> windows);
    // conversion de  std::unordered_map<xcb_window_t, Window> windows en std::vector<Window>
    std::vector<Window> convertWindowsToVector(std::unordered_map<xcb_window_t, Window> windows);
    // Valeur booléenne qui indique si le gestionnaire de fenêtres est en mode configuration
    bool configurationMode{false};
    // Fonction de capture pour optimiser les transitions
    xcb_pixmap_t captureWindow(xcb_window_t windowId);
    // Gestion des logs
    LogManager *logManager;
    // Changement de la taille d'une fenêtre
    void resizeWindow(xcb_window_t window, uint32_t width,uint32_t height);
    // Déplacement de la fenêtre
    void moveWindow(xcb_window_t window, uint32_t x,uint32_t y);

public:
    int countWindow{0};
    int countDownWindow{0};
    XcbManager(LogManager &log);
    ~XcbManager();
    void flush();
    xcb_screen_t *getScreen();
    // Fonction pour récupérer la connexion au serveur X
    xcb_connection_t *getConnection();
    // Fonction pour récupérer les fonctionnalités EWMH
    EWMHManager *getEWMHManager();
    // Fonction qui donne la configuration des écrans
    std::vector<Display> getDisplayConfiguration();
    // Fonction qui donne la configuration des écrans pour les sockets
    std::string getDisplayConfigurationForSocket();
    // Fonction qui donne la configuration d'une fenêtre
    Window getWindow(xcb_window_t window);
    // Liste des fenêtres gérées par le gestionnaire de fenêtres
    std::unordered_map<xcb_window_t, Window> windows;
    // Fonction qui centre une fenêtre A par rapport à une fenêtre B
    void centerWindow(xcb_window_t windowA, xcb_window_t windowB);
    // Buffer de fenêtres à venir avec comme clef le nom de la classe et comme valeur un tableau de configuration de fenêtres
    std::unordered_map<std::string, std::vector<ConfigurationWindow>> windowsToCome;
    // Fonction qui applique la configuration géométrique d'une fenêtre
    void applyGeometryConfiguration(ConfigurationWindow configuration,bool rewrite);
    void applyGeometryConfiguration(std::string jsonConfigurationWindow);
    // Fonction qui toggle le mode plein écran d'une fenêtre
    void toggleFullscreen(xcb_window_t window);
    // Fonction qui ajoute le mode plein écran pour une fenêtre
    void addFullscreen(xcb_window_t window);
    // Fonction qui enlève le mode plein écran pour une fenêtre
    void removeFullscreen(xcb_window_t window);
    // Fonction qui met la fenêtre toujours au dessus
    void addAbove(xcb_window_t window);
    // fonction qui enlève toujours au dessus de la fenêtre
    void removeAbove(xcb_window_t window);
    // fonction qui toggle l'atôme above
    void toggleAbove(xcb_window_t window);
    // Fonction qui met la fenêtre toujours au dessous
    void addBelow(xcb_window_t window);
    // fonction qui enlève toujours au dessous de la fenêtre
    void removeBelow(xcb_window_t window);
    // fonction qui toggle l'atôme bellow
    void toggleBelow(xcb_window_t window);
    // fonction qui ajoute le state hidden
    void addHidden(xcb_window_t window);
    // fonction qui retire le state hidden
    void removeHidden(xcb_window_t window);
    // fonction qui toggle le state hidden
    void toggleHidden(xcb_window_t window);
    // Fonction qui parcours windows pour récupérer la fenêtre qui a le focus
    xcb_window_t getActiveWindow();
    // Fonction qui donne le focus à une fenêtre
    void setActiveWindow(xcb_window_t window);
    // Fonction qui met la fenêtre active en haut a gauche de l'écran sur lequel elle est
    void setWindowOnTopLeft(xcb_window_t window);
    // Fonction qui met la fenêtre active en haut a droite de l'écran sur lequel elle est
    void setWindowOnTopRight(xcb_window_t window);
    // Fonction qui met la fenêtre active en bas a gauche de l'écran sur lequel elle est
    void setWindowOnBottomLeft(xcb_window_t window);
    // Fonction qui met la fenêtre active en bas a droite de l'écran sur lequel elle est
    void setWindowOnBottomRight(xcb_window_t window);
    // Fonction qui met la fenêtre active a gauche de l'écran sur lequel elle est
    void setWindowOnLeft(xcb_window_t window);
    // Fonction qui met la fenêtre active a droite de l'écran sur lequel elle est
    void setWindowOnRight(xcb_window_t window);
    // Fonction qui met la fenêtre active en haut de l'écran sur lequel elle est
    void setWindowOnTop(xcb_window_t window);
    // Fonction qui met la fenêtre active en bas de l'écran sur lequel elle est
    void setWindowOnBottom(xcb_window_t window);
    // Fonction qui met la fenêtre active sur l'écran suivant de manière circulaire et dans les mêmes proportions de taille et de position
    void setWindowOnNextScreen(xcb_window_t window);
    // Fonction qui met la fenêtre soit a l'index le plus haut soit toujours au dessus
    void setWindowOnAbove(xcb_window_t window);
    // Fonction qui met la fenêtre a l'index le plus bas ou soit toujours en dessous
    void setWindowOnBellow(xcb_window_t window);
    // Fonction qui met une fenêtre en transparence avec une valeur de 0 à 1 de transparence avec l'atome _NET_WM_WINDOW_OPACITY
    void setWindowOpacity(xcb_window_t window, uint8_t opacity);
    // Foction qui renvoie pour la communication Socket un json de vector<Windows> de toutes les fenêtres gérée par le WindowManager
    std::string getWindowsForSocket();
    // Fonction pour mettre le type EWMH en EWMH._NET_WM_WINDOW_TYPE_DESKTOP
    void setToDesktop(int id);
    // Fonction pour mettre le type EWMH en EWMH._NET_WM_WINDOW_TYPE_DOCK
    void setToDock(int id);
    // Fonction pour appliquer le stack de pile
    void restack_windows();
    // Fonction pour mettre la hauteur d'une fenetre a x pixel
    void setDockHeight(uint32_t height);
    // Fonction de synchronisation
    void xcb_aux_sync (xcb_connection_t *c);
    // Fonction pour envoyer une demande de fermeture à l'application
    void sendDeleteWindow(xcb_window_t window);
    // Fonction pour la destruction d'une fenêtre
    void deleteWindow(xcb_window_t window);
    // Fonction pour vérifier si l'application supporte _wm_delete pour la fermeture
    bool check_wm_delete_support(xcb_window_t window);
};

