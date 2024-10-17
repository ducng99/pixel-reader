#include "./held_key_tracker.h"

#include <SDL2/SDL.h>

HeldKeyTracker::HeldKeyTracker(std::vector<SDL_GameControllerButton> keycodes)
    : keycodes(keycodes),
      held_times(keycodes.size(), 0),
      held_keys()
{
}

HeldKeyTracker::~HeldKeyTracker()
{
}

void HeldKeyTracker::on_keypress(SDL_GameControllerButton button)
{
    held_keys[button] = true;
}

void HeldKeyTracker::on_keyrelease(SDL_GameControllerButton button)
{
    held_keys[button] = false;
}

void HeldKeyTracker::accumulate(uint32_t ms)
{
    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        auto held_key_it = held_keys.find(*key_it);

        if (held_key_it != held_keys.end() && held_key_it->second)
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

bool HeldKeyTracker::for_longest_held(const std::function<void(SDL_GameControllerButton, uint32_t)> &callback)
{
    uint32_t longest_time = 0;
    SDL_GameControllerButton longest_key = SW_BTN_UNKNOWN;

    auto key_it = keycodes.begin();
    auto time_it = held_times.begin();
    while (key_it != keycodes.end())
    {
        SDL_GameControllerButton key = *key_it;
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
