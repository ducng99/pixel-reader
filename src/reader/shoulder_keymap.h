#ifndef SHOULDER_KEYMAP_H_
#define SHOULDER_KEYMAP_H_

#include "sys/keymap.h"
#include <SDL2/SDL_gamecontroller.h>
#include <cstdint>
#include <string>

std::string get_valid_shoulder_keymap(const std::string &keymap);
std::string get_prev_shoulder_keymap(const std::string &keymap);
std::string get_next_shoulder_keymap(const std::string &keymap);

std::pair<SW_BTN_TYPE, SW_BTN_TYPE> get_shoulder_keymap_lr(const std::string &keymap);
const std::string &get_shoulder_keymap_display_name(const std::string &keymap);

#endif
