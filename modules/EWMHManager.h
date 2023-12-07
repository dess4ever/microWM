#pragma once
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb.h>
#include <vector>
#include <iostream>
#include <xcb/xcb.h>
#include "configurationWindow.h"
#include "window.h"

class EWMHManager
{
    private:
            xcb_intern_atom_cookie_t *EWMHCookie;
            xcb_connection_t *connection;
            void addAllowedActionsAtomToRootWindow(std::vector<xcb_atom_t> atoms);
            // Fonction qui donne le type d'une fenêtre
            EWMHtype getWindowType(xcb_window_t window);
            // Fonction pour obtenir les hints WM_NORMAL_HINTS d'une fenêtre
            std::vector<WMNormalHints> getWMNormalHints(xcb_connection_t* connection, xcb_window_t window);
            EWMHSTATES atomToEWMHSTATES(xcb_atom_t atom);
        
    public:
            xcb_ewmh_connection_t EWMH;
            EWMHManager(xcb_connection_t *connection);
            ~EWMHManager();
            // Fonction qui met à jour _net_active_window
            void setActiveWindow(xcb_window_t window);
            // Buffer de fenêtres gérées par le WM
            Window getWindow(xcb_window_t window);
            // Fonction qui affiche les propriétés d'une fenêtre
            void printWindowProperties(Window window);
};