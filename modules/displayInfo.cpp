#include "displayInfo.h"

DisplayInfo::DisplayInfo(xcb_connection_t *connection)
{
    xcb_randr_get_screen_resources_current_reply_t *screen = xcb_randr_get_screen_resources_current_reply(
        connection,
        xcb_randr_get_screen_resources_current(connection, xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root), NULL);
    if (screen == NULL)
    {
        std::cerr << "Erreur: Impossible de récupérer les informations sur les écrans\n";
        exit(-1);
    }
    else
    {
        xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_current_outputs(screen);
        int nb_outputs = xcb_randr_get_screen_resources_current_outputs_length(screen);
        for (int i = 0; i < nb_outputs; i++)
        {
            xcb_randr_get_output_info_reply_t *output = xcb_randr_get_output_info_reply(
                connection,
                xcb_randr_get_output_info(connection, outputs[i], XCB_CURRENT_TIME),
                NULL);
            if (output == NULL)
            {
                std::cerr << "Erreur: Impossible de récupérer les informations sur l'écran\n";
            }
            else
            {
                xcb_randr_get_crtc_info_reply_t *crtc = xcb_randr_get_crtc_info_reply(
                    connection,
                    xcb_randr_get_crtc_info(connection, output->crtc, XCB_CURRENT_TIME),
                    NULL);
                if (crtc != NULL)
                {
                    Display screen;
                    screen.width = crtc->width;
                    screen.height = crtc->height;
                    screen.x = crtc->x;
                    screen.y = crtc->y;
                    screens.push_back(screen);
                }
                free(crtc);
            }
            free(output);
        }
    }
    free(screen);
}

DisplayInfo::~DisplayInfo()
{
    // On se déconnecte du serveur X
    std::cout << "Déconnexion du serveur X\n";
    xcb_disconnect(DisplayInfo::connection);
}

void DisplayInfo::print()
{
    // On affiche les informations sur les écrans
    for (int i = 0; i < screens.size(); i++)
    {
        std::cout << "Informations sur l'écran " << i << " récupérées:\n";
        std::cout << "\tRésolution de l'écran " << screens[i].width << "x" << screens[i].height << "\n";
        std::cout << "\tPosition de l'écran " << screens[i].x << "," << screens[i].y << "\n";
    }
}

std::vector<Display> DisplayInfo::getScreens()
{
    return screens;
}
