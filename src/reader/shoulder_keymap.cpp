#include "./shoulder_keymap.h"

#include "sys/keymap.h"

#include <vector>

namespace
{

struct KeyMap
{
    std::string display_name;
    std::pair<SW_BTN_TYPE, SW_BTN_TYPE> lr_binding;
};

const std::vector<std::pair<std::string, KeyMap>> keymaps = {
    {"LR", {"LR", {SW_BTN_L1, SW_BTN_R1}}},
    {"L2R2", {"L2R2", {SW_BTN_L2, SW_BTN_R2}}},
};

int get_keymap_index(const std::string &name)
{
    for (uint32_t i = 0; i < keymaps.size(); i++)
    {
        if (keymaps[i].first == name)
        {
            return i;
        }
    }

    return 0;
}

} // namespace

std::string get_valid_shoulder_keymap(const std::string &keymap)
{
    return keymaps[get_keymap_index(keymap)].first;
}

std::string get_prev_shoulder_keymap(const std::string &keymap)
{
    int index = get_keymap_index(keymap);
    index = (index + keymaps.size() - 1) % keymaps.size();

    return keymaps[index].first;
}

std::string get_next_shoulder_keymap(const std::string &keymap)
{
    int index = get_keymap_index(keymap);
    index = (index + 1) % keymaps.size();

    return keymaps[index].first;
}

std::pair<SW_BTN_TYPE, SW_BTN_TYPE> get_shoulder_keymap_lr(const std::string &keymap)
{
    return keymaps[get_keymap_index(keymap)].second.lr_binding;
}

const std::string &get_shoulder_keymap_display_name(const std::string &keymap)
{
    return keymaps[get_keymap_index(keymap)].second.display_name;
}
