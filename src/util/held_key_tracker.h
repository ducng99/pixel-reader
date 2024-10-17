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
    std::vector<SW_BTN_TYPE> keycodes;
    std::vector<uint32_t> held_times;
    std::unordered_map<SW_BTN_TYPE, bool> held_keys;

public:
    HeldKeyTracker(std::vector<SW_BTN_TYPE> keycodes);
    virtual ~HeldKeyTracker();

    void on_keypress(SW_BTN_TYPE button);
    void on_keyrelease(SW_BTN_TYPE button);
    void accumulate(uint32_t ms);
    bool for_longest_held(const std::function<void(SW_BTN_TYPE, uint32_t)> &callback);
};

#endif
