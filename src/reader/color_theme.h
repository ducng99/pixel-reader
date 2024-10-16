#ifndef COLOR_THEME_H_
#define COLOR_THEME_H_

#include <SDL2/SDL_video.h>

struct ColorTheme
{
    SDL_Color background;
    SDL_Color main_text;
    SDL_Color secondary_text;
    SDL_Color highlight_background;
    SDL_Color highlight_text;
};

#endif
