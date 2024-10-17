#ifndef KEYMAP_H_
#define KEYMAP_H_

#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_gamecontroller.h>

#define SW_BTN_UNKNOWN  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_INVALID
#define SW_BTN_UP       SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP
#define SW_BTN_DOWN     SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN
#define SW_BTN_LEFT     SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT
#define SW_BTN_RIGHT    SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT
#define SW_BTN_A        SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A
#define SW_BTN_B        SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B
#define SW_BTN_X        SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X
#define SW_BTN_Y        SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y
#define SW_BTN_L1       SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define SW_BTN_R1       SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
#define SW_BTN_L2       SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK
#define SW_BTN_R2       SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK
#define SW_BTN_SELECT   SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK
#define SW_BTN_START    SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START
#define SW_BTN_POWER    SDL_Scancode::SDL_SCANCODE_UNKNOWN

#endif
