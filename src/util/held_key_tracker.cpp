#include "./held_key_tracker.h"

#include <SDL2/SDL.h>

HeldKeyTracker::HeldKeyTracker(std::vector<SDL_Scancode> keycodes)
    : keycodes(keycodes),
      held_times(keycodes.size(), 0)
{
}

HeldKeyTracker::~HeldKeyTracker()
{
}

void HeldKeyTracker::accumulate(uint32_t ms)
{
    const Uint8 *scancode = SDL_GetKeyboardState(nullptr);

    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        if (scancode[*key_it])
        {
            *time_it += ms;
        }
        else
        {
            *time_it = 0;
        }

        ++key_it;
        ++time_it;
    }
}

bool HeldKeyTracker::for_longest_held(const std::function<void(SDL_Scancode, uint32_t)> &callback)
{
    uint32_t longest_time = 0;
    SDL_Scancode longest_key = SDL_Scancode::SDL_SCANCODE_UNKNOWN;

    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        SDL_Scancode key = *key_it;
        uint32_t time = *time_it;
        if (time > longest_time)
        {
            longest_time = time;
            longest_key = key;
        }

        ++key_it;
        ++time_it;
    }

    if (longest_time != 0)
    {
        callback(longest_key, longest_time);
        return true;
    }

    return false;
}
