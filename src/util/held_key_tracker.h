#ifndef HELD_KEY_TRACKER_H_
#define HELD_KEY_TRACKER_H_

#include <SDL2/SDL_scancode.h>

#include <cstdint>
#include <functional>
#include <vector>

class HeldKeyTracker {
    std::vector<SDL_Scancode> keycodes;
    std::vector<uint32_t> held_times;

public:
    HeldKeyTracker(std::vector<SDL_Scancode> keycodes);
    virtual ~HeldKeyTracker();

    void accumulate(uint32_t ms);
    bool for_longest_held(const std::function<void(SDL_Scancode, uint32_t)> &callback);
};

#endif
