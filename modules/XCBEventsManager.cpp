#include "XCBEventsManager.h"

XCBEventsManager::XCBEventsManager(LogManager &logManager)
{
    this->logManager=&logManager;
    xcbManager = new XcbManager(logManager);
}

XCBEventsManager::~XCBEventsManager()
{
    delete xcbManager;
}

void XCBEventsManager::grabNumericKeyboard()
{
    xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(xcbManager->getConnection());

    for (int i = 0; i < 10; i++)
    {
        xcb_keycode_t *keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_KP_0 + i);
        xcb_grab_key(xcbManager->getConnection(), 1, xcbManager->getScreen()->root, XCB_MOD_MASK_ANY, keycodesPtr[0], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    }
    xcb_keycode_t *keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_KP_Add);
    xcb_grab_key(xcbManager->getConnection(), 1, xcbManager->getScreen()->root, XCB_MOD_MASK_ANY, keycodesPtr[0], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    
    keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_KP_Subtract);
    xcb_grab_key(xcbManager->getConnection(), 1, xcbManager->getScreen()->root, XCB_MOD_MASK_ANY, keycodesPtr[0], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_Delete);
    xcb_grab_key(xcbManager->getConnection(), 1, xcbManager->getScreen()->root, XCB_MOD_MASK_ANY, keycodesPtr[0], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);


    xcb_key_symbols_free(keysyms);
    xcb_flush(xcbManager->getConnection());
}

void XCBEventsManager::ungrabNumericKeyboard()
{
    xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(xcbManager->getConnection());

    for (int i = 0; i < 10; i++)
    {
        xcb_keycode_t *keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_KP_0 + i);
        xcb_ungrab_key(xcbManager->getConnection(), keycodesPtr[0], xcbManager->getScreen()->root, XCB_MOD_MASK_ANY);
    }

    xcb_keycode_t *keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_KP_Add);
    xcb_ungrab_key(xcbManager->getConnection(), 1, xcbManager->getScreen()->root, XCB_MOD_MASK_ANY);
    
    keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_KP_Subtract);
    xcb_ungrab_key(xcbManager->getConnection(), 1, xcbManager->getScreen()->root, XCB_MOD_MASK_ANY);

    keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_Delete);
    xcb_ungrab_key(xcbManager->getConnection(), 1, xcbManager->getScreen()->root, XCB_MOD_MASK_ANY);


    xcb_key_symbols_free(keysyms);
    xcb_flush(xcbManager->getConnection());
}

xcb_generic_event_t *XCBEventsManager::getEvent()
{
    return xcb_poll_for_event(xcbManager->getConnection());
}

int XCBEventsManager::getFileDescriptor()
{
    return xcb_get_file_descriptor(xcbManager->getConnection());
}

void XCBEventsManager::configurationRequest()
{
    // Récupérer la requête de configuration
    auto* configure_request = reinterpret_cast<xcb_configure_request_event_t*>(the_events);

    // Préparation des valeurs pour xcb_configure_window
    const uint32_t values[] = {
        static_cast<u_int32_t>(configure_request->x),
        static_cast<u_int32_t>(configure_request->y),
        static_cast<u_int32_t>(configure_request->width),
        static_cast<u_int32_t>(configure_request->height),
        static_cast<uint32_t>(configure_request->border_width)
    };

    // Envoyer la requête de configuration au serveur X
    xcb_void_cookie_t cookie = xcb_configure_window_checked(
        xcbManager->getConnection(),
        configure_request->window,
        XCB_CONFIG_WINDOW_X |
        XCB_CONFIG_WINDOW_Y |
        XCB_CONFIG_WINDOW_WIDTH |
        XCB_CONFIG_WINDOW_HEIGHT |
        XCB_CONFIG_WINDOW_BORDER_WIDTH,
        values
    );

    // Vérifier les erreurs lors de l'envoi de la requête de configuration
    xcb_generic_error_t *error = xcb_request_check(xcbManager->getConnection(), cookie);
    if (error)
    {
        logManager->addLog(Log("Erreur lors de la configuration de la fenêtre: " + std::to_string(error->error_code), LogSeverity::Error, "XCBEventsManager->configurationRequest"));
        free(error);
    }

    // Synchroniser le serveur X après la configuration pour s'assurer que les changements sont appliqués
    xcbManager->xcb_aux_sync(xcbManager->getConnection());

    // Vider le tampon de commandes
    xcb_flush(xcbManager->getConnection());
}


void XCBEventsManager::destroyWindowNotify()
{
    // Si la fenêtre étair gérée par le gestionnaire de fenêtre alors je la supprime
    xcb_destroy_notify_event_t *destroy_notify = (xcb_destroy_notify_event_t *)the_events;
    xcb_window_t window = destroy_notify->window;
    if (xcbManager->windows.find(window) != xcbManager->windows.end())
    {
        xcbManager->windows.erase(window);
    }
    // on rend la main au serveur X
    xcb_flush(xcbManager->getConnection());
}

void XCBEventsManager::unmapWindowNotify()
{
    // Si la fenêtre est gérée par le gestionnaire de fenêtres j'ajoute l'atome EWMH_STATE_HIDDEN au tableau des états de la fenêtre
    xcb_unmap_notify_event_t *unmap_notify = (xcb_unmap_notify_event_t *)the_events;
    xcb_window_t window = unmap_notify->window;
    if (xcbManager->windows.find(window) != xcbManager->windows.end())
    {
        xcbManager->windows[window].atoms.push_back(EWMH_STATE_HIDDEN);
    }
    // on restaque les fenêtres
    xcbManager->restack_windows();
}



void XCBEventsManager::validateMapRequest()
{
    xcb_map_request_event_t *map_request = (xcb_map_request_event_t *)the_events;
    xcb_window_t window = map_request->window;
    // Est-ce que la fenêtre est déjà gerée par le gestionnaire de fenêtre ?
    if (xcbManager->windows.find(window) != xcbManager->windows.end())
    {
        // Si oui, j'enlève l'atôme _NET_WM_STATE_HIDDEN de la fenêtre
        xcbManager->windows[window].atoms.erase(std::remove(xcbManager->windows[window].atoms.begin(), xcbManager->windows[window].atoms.end(), EWMH_STATE_HIDDEN), xcbManager->windows[window].atoms.end());
    }
    // Sinon je l'ajoute à la liste des fenêtres gérées par le gestionnaire de fenêtres
    else if(window!=0)
    {
        // je récupère les informations sur la fenêtre

        Window ewmhWindow=xcbManager->getWindow(window);
        ewmhWindow.order=xcbManager->countWindow;
        xcbManager->countWindow+=1;
        // J'ajoute la fenêtre à la liste des fenêtres gérées par le gestionnaire de fenêtres
        xcbManager->windows.insert(std::make_pair(window, ewmhWindow));
        // Je regarde si la fenêtre est dans windowsToCome
        if(xcbManager->windowsToCome.find(ewmhWindow.normalConfiguration.windowClass) != xcbManager->windowsToCome.end())
        {
            // Je pop la première configuration de la fenêtre
            ConfigurationWindow configurationWindow = xcbManager->windowsToCome[ewmhWindow.normalConfiguration.windowClass].back();
            xcbManager->windowsToCome[ewmhWindow.normalConfiguration.windowClass].pop_back();
            // J'applique la configuration à la fenêtre
            xcbManager->applyGeometryConfiguration(configurationWindow,true);
            // Si il n'y a plus de configuration pour cette fenêtre je la supprime de la liste des fenêtres à venir
            if(xcbManager->windowsToCome[ewmhWindow.normalConfiguration.windowClass].size()==0)
            {
                xcbManager->windowsToCome.erase(ewmhWindow.normalConfiguration.windowClass);
            }
        }
        else
        {
            // Si la fenetre est transiente je la centre par rapport à son parent et je la met au dessus de la pile (en rapport avec son parent)
            if(ewmhWindow.transientFor!=0)
            {
                xcbManager->centerWindow(window, ewmhWindow.transientFor);
            }
        }
        // Je met le focus sur la fenêtre
        xcbManager->setActiveWindow(window);
    }

    // Je mappe la fenêtre
    xcb_map_window(xcbManager->getConnection(), window);
    xcb_flush(xcbManager->getConnection());
    // Je m'abonne aux évenements enter et leave à la fenêtre 
    uint32_t events[]={XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW};
    xcb_change_window_attributes(xcbManager->getConnection(), window, XCB_CW_EVENT_MASK, events);
    // Je restack les fenêtres
    xcbManager->restack_windows();
}

XcbManager *XCBEventsManager::getXcbManager()
{
    return xcbManager;
}

void XCBEventsManager::expose()
{
    xcb_expose_event_t *event=(xcb_expose_event_t*)the_events;
    // Demandez à l'application de redessiner la partie exposée de la fenêtre
    xcb_clear_area(xcbManager->getConnection(), XCB_NONE, event->window, 
                   event->x, event->y, event->width, event->height);
    xcb_flush(xcbManager->getConnection());

}

auto XCBEventsManager::get_atom_name(xcb_get_atom_name_reply_t* (*reply_func)(xcb_connection_t*, xcb_get_atom_name_cookie_t, xcb_generic_error_t**), xcb_connection_t* conn, xcb_atom_t atom)
{
    auto atom_reply = reply_func(conn, xcb_get_atom_name(conn, atom), NULL);
    std::string atom_name = xcb_get_atom_name_name(atom_reply);
    free(atom_reply);
    return atom_name;

}


template <typename T>
auto XCBEventsManager::check_and_apply(const std::string& full_atom_name, const std::string& query_atom_name, T action){
    return [&, action](xcb_window_t win){
        if(full_atom_name.find(query_atom_name)!=std::string::npos){
            action(win);
            return true;
        }
        return false;
    };
}

void XCBEventsManager::handleEvent()
{
    while (the_events = getEvent())
    {				
        switch (the_events->response_type & ~0x80)
        {
        case XCB_MAP_REQUEST:
        {
            logManager->addLog(Log("Nouvel évenement MapRequest",LogSeverity::Info,"XCBEventManager->handleEvent"));
            validateMapRequest();
            break;
        }
        case XCB_CONFIGURE_REQUEST:
        {
            logManager->addLog(Log("Nouvel évenement Configure Request",LogSeverity::Info,"XCBEventManager->handleEvent"));
            configurationRequest();
            break;
        }
        case XCB_DESTROY_NOTIFY:
            logManager->addLog(Log("Nouvel évenement Destroy Notify",LogSeverity::Info,"XCBEventManager->handleEvent"));
            destroyWindowNotify();
            break;
        case XCB_UNMAP_NOTIFY:
            unmapWindowNotify();
            break;
        case XCB_CLIENT_MESSAGE:
        {
            xcb_client_message_event_t *client_message = (xcb_client_message_event_t *)the_events;
            std::string atom_name = get_atom_name(&xcb_get_atom_name_reply, xcbManager->getConnection(), client_message->type);
            if(atom_name =="_NET_WM_STATE"){
                std::string for_atom_name = get_atom_name(&xcb_get_atom_name_reply, xcbManager->getConnection(), client_message->data.data32[1]);
                xcb_window_t window = client_message->window;
                bool processed = false;

                using action_func = std::function<void(xcb_window_t)>;
                using check_apply_func = std::function<bool(xcb_window_t)>;
                
                std::map<int, std::map<std::string, action_func>> action_map {
                    { 
                    0, // suppression
                    {
                        { "_NET_WM_STATE_FULLSCREEN", [&](xcb_window_t win){ xcbManager->removeFullscreen(win); } },                   
                        { "_NET_WM_STATE_ABOVE", [&](xcb_window_t win){ xcbManager->removeAbove(win); } },   
                        { "_NET_WM_STATE_BELOW", [&](xcb_window_t win){ xcbManager->removeBelow(win); } },
                        { "_NET_WM_STATE_HIDDEN", [&](xcb_window_t win){ xcbManager->removeHidden(win); } },
                        { "_NET_WM_STATE_MAXIMIZED_VERT", [&](xcb_window_t win){ xcbManager->removeMaximizedVert(win); } },
                        { "_NET_WM_STATE_MAXIMIZED_HORIZ", [&](xcb_window_t win){ xcbManager->removeMaximizedHoriz(win); } }  
                    },
                    },
                    {
                    1, // ajout
                    {
                        { "_NET_WM_STATE_FULLSCREEN", [&](xcb_window_t win){ xcbManager->addFullscreen(win); } },
                        { "_NET_WM_STATE_ABOVE", [&](xcb_window_t win){ xcbManager->addAbove(win); } },
                        { "_NET_WM_STATE_BELOW", [&](xcb_window_t win){ xcbManager->addBelow(win); } },
                        { "_NET_WM_STATE_HIDDEN", [&](xcb_window_t win){ xcbManager->addHidden(win); } },
                        { "_NET_WM_STATE_MAXIMIZED_VERT", [&](xcb_window_t win){ xcbManager->addMaximizedVert(win); } },
                        { "_NET_WM_STATE_MAXIMIZED_HORIZ", [&](xcb_window_t win){ xcbManager->addMaximizedHoriz(win); } }                          
                    },
                    },
                    {
                    2, // changement
                    {
                        { "_NET_WM_STATE_FULLSCREEN", [&](xcb_window_t win){ xcbManager->toggleFullscreen(win); } },
                        { "_NET_WM_STATE_ABOVE", [&](xcb_window_t win){ xcbManager->toggleAbove(win); } },
                        { "_NET_WM_STATE_BELOW", [&](xcb_window_t win){ xcbManager->toggleBelow(win); } },
                        { "_NET_WM_STATE_HIDDEN", [&](xcb_window_t win){ xcbManager->toggleHidden(win); } },
                        { "_NET_WM_STATE_MAXIMIZED_VERT", [&](xcb_window_t win){ xcbManager->toggleMaximizedVert(win); } },
                        { "_NET_WM_STATE_MAXIMIZED_HORIZ", [&](xcb_window_t win){ xcbManager->toggleMaximizedHoriz(win); } }                         
                    },
                    },
                };

                auto it_map = action_map.find(client_message->data.data32[0]);
                if(it_map != action_map.end()){
                    for (const auto& [atome, action] : it_map->second){
                        check_apply_func check_func = check_and_apply(for_atom_name, atome, action);
                        processed = check_func(window);
                        if(processed) break;
                    }
                }

                if(!processed){
                    logManager->addLog(Log("Gestion de l'atome _NET_WM_STATE non gérée"+ for_atom_name, LogSeverity::Warning, "XCBEventManager->handleEvent"));
                }
            }
            else
            {
                    logManager->addLog(Log("Gestion non gérée pour:"+ atom_name, LogSeverity::Warning, "XCBEventManager->handleEvent"));
            }
        }
        break;
        case XCB_ENTER_NOTIFY:
        {
            logManager->addLog(Log("Nouvel évenement EnterNotify",LogSeverity::Info,"XCBEventManager->handleEvent"));
            xcb_enter_notify_event_t *enter_notify = (xcb_enter_notify_event_t *)the_events;
            xcb_window_t window = enter_notify->event;
            // Si la fenêtre n'est pas root
            if(window!=0)
            {
                // Je donne le focus à la fenêtre
                xcbManager->setActiveWindow(window);
            }
        }
            break;
        case XCB_LEAVE_NOTIFY:
        {
            xcb_leave_notify_event_t *leaveNotify=(xcb_leave_notify_event_t *)the_events;
            logManager->addLog(Log("Nouvel évenement LeaveNotify",LogSeverity::Info,"XCBEventManager->handleEvent"));
        }
            break;
        case XCB_PROPERTY_NOTIFY:
            logManager->addLog(Log("Nouvel évenement PropretyNotify",LogSeverity::Info,"XCBEventManager->handleEvent"));
            break;
        case XCB_CONFIGURE_NOTIFY:
            logManager->addLog(Log("Nouvel évenement ConfigureNotify",LogSeverity::Info,"XCBEventManager->handleEvent"));
        case XCB_EXPOSE:
        {
            logManager->addLog(Log("Nouvel évenement Expose",LogSeverity::Info,"XCBEventManager->handleEvent"));
            expose();
        }
            break;
        case XCB_KEY_PRESS:
        switch (the_events->pad0)
        {
        case 96: // touche F12
            {
            }
            break;
        case 133: // touche Windows
                //xcbManager->showConfigurationInterface();
            break;
        case 77: // touche NumLock
            numlock_active = !numlock_active;
            if (numlock_active)
            {
                grabNumericKeyboard();
            }
            else
            {
                ungrabNumericKeyboard();
            }
            break;
        case 79:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnTopLeft(window);
            }
            break;
        case 80:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnTop(window);
            }
            break;
        case 81:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnTopRight(window);
            }
            break;
        case 82:
            {
                xcbManager->setWindowOnBellow(xcbManager->getActiveWindow());
                logManager->addLog(Log("Mise de la fenêtre "+std::to_string(xcbManager->getActiveWindow())+" au dessous",LogSeverity::Info,"XCBEventsManager->handleEvents"));      
            }
        break;
        case 83:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnLeft(window);
            }
            break;
        case 84:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->toggleFullscreen(window);
            }
            break;
        case 85:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnRight(window);
            }
            break;
        case 86:
            {
                xcbManager->setWindowOnAbove(xcbManager->getActiveWindow());
                logManager->addLog(Log("Mise de la fenêtre "+std::to_string(xcbManager->getActiveWindow())+" au dessus",LogSeverity::Info,"XCBEventsManager->handleEvents"));      
            }
            break;
        case 87:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnBottomLeft(window);
            }
            break;
        case 88:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnBottom(window);
            }
            break;
        case 89:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnBottomRight(window);
            }
            break;
        case 90:
            {
                // Je récupère la fenêtre qui a le focus en parcourant les fenêtres gérées par le gestionnaire de fenêtres
                xcb_window_t window = xcbManager->getActiveWindow();
                xcbManager->setWindowOnNextScreen(window);
            }
            break;
        case 119:
            {
                xcb_window_t window = xcbManager->getActiveWindow();
                logManager->addLog(Log("Demande de fermeture pour la fenêtre "+std::to_string(window),LogSeverity::Warning,"XCBEventManager->handleEvent"));
                xcbManager->sendDeleteWindow(window);
            }
            break;
        default:
            logManager->addLog(Log("Autre Touche appuyéé:" + std::to_string(the_events->pad0),LogSeverity::Info,"XCBEventManager->handleEvent"));
            break;
        }

        break;
    default:
        break;
    }
    free(the_events);
    }
}

