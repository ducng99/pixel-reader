#ifndef HELD_KEY_TRACKER_H_
#define HELD_KEY_TRACKER_H_

#include <SDL2/SDL_gamecontroller.h>

#include <cstdint>
#include <functional>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include "../sys/keymap.h"

class HeldKeyTracker {
    std::vector<SDL_GameControllerButton> keycodes;
    std::vector<uint32_t> held_times;
    std::unordered_map<SDL_GameControllerButton, bool> held_keys;

public:
    HeldKeyTracker(std::vector<SDL_GameControllerButton> keycodes);
    virtual ~HeldKeyTracker();

    void on_keypress(SDL_GameControllerButton button);
    void on_keyrelease(SDL_GameControllerButton button);
    void accumulate(uint32_t ms);
    bool for_longest_held(const std::function<void(SDL_GameControllerButton, uint32_t)> &callback);
};

#endif
