#include "./settings_view.h"
#include "./token_view/token_view_styling.h"

#include "reader/color_theme_def.h"
#include "reader/config.h"
#include "reader/draw_modal_border.h"
#include "reader/font_catalog.h"
#include "reader/settings_store.h"
#include "reader/shoulder_keymap.h"
#include "reader/system_styling.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include "util/sdl_utils.h"

#include <filesystem>
#include <algorithm>

SettingsView::SettingsView(
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    std::string font_name
) : font_name(font_name),
    sys_styling(sys_styling),
    token_view_styling(token_view_styling),
    styling_sub_id(sys_styling.subscribe_to_changes([this](SystemStyling::ChangeId) {
        needs_render = true;
    })),
    num_menu_items(5)
{
}

SettingsView::~SettingsView()
{
    sys_styling.unsubscribe_from_changes(styling_sub_id);
}

bool SettingsView::render(SDL_Surface *dest_surface, bool force_render)
{
    if (needs_render || force_render)
    {
        TTF_Font *sys_font = cached_load_font(font_name, sys_styling.get_font_size());
        TTF_Font *user_font = sys_styling.get_loaded_font();
        const auto &theme = sys_styling.get_loaded_color_theme();

        constexpr int style_normal = 0;
        constexpr int style_hl = 1;
        constexpr int style_label = 2;

        auto render_text = [&](const char *str, int style, TTF_Font *font = nullptr) {
            return surface_unique_ptr { TTF_RenderUTF8_Shaded(
                font ? font : sys_font,
                str,
                style == style_normal ?
                    theme.main_text :
                    (style == style_hl ? theme.highlight_text : theme.secondary_text),
                style == style_normal ?
                    theme.background :
                    (style == style_hl ? theme.highlight_background : theme.background)
            ) };
        };

        auto left_arrow = render_text("◂", style_hl);
        auto right_arrow = render_text("▸", style_hl);

        auto theme_label = render_text("Theme:", style_label);
        auto theme_value = render_text(
            sys_styling.get_color_theme().c_str(),
            line_selected == 0 ? style_hl : style_normal
        );

        auto font_size_label = render_text("Font size:", style_label);
        auto font_size_value = render_text(
            std::to_string(sys_styling.get_font_size()).c_str(),
            line_selected == 1 ? style_hl : style_normal
        );

        auto line_padding_label = render_text("Line padding:", style_label);
        auto line_padding_value = render_text(
            std::to_string(sys_styling.get_line_padding()).c_str(),
            line_selected == 2 ? style_hl : style_normal
        );

        auto font_name_label = render_text("Font:", style_label);
        auto font_name_value = render_text(
            std::filesystem::path(sys_styling.get_font_name()).filename().stem().string().c_str(),
            line_selected == 3 ? style_hl : style_normal,
            user_font
        );

        auto shoulder_keymap_label = render_text("Shoulder keymap:", style_label);
        auto shoulder_keymap_value = render_text(
            get_shoulder_keymap_display_name(
                sys_styling.get_shoulder_keymap()
            ).c_str(),
            line_selected == 4 ? style_hl : style_normal
        );

        auto progress_label = render_text("Progress:", style_label);
        auto progress_value = render_text(
            token_view_styling.get_progress_reporting() == ProgressReporting::CHAPTER_PERCENT ?
            "Chapter %" :
            "Book %",
            line_selected == 5 ? style_hl : style_normal
        );

        Uint16 content_w;
        {
            int arrow_w = left_arrow->w + right_arrow->w;
            std::vector<int> widths {
                theme_label->w,
                theme_value->w + arrow_w,
                font_size_label->w,
                font_size_value->w + arrow_w,
                line_padding_label->w,
                line_padding_value->w + arrow_w,
                font_name_label->w,
                font_name_value->w + arrow_w,
                shoulder_keymap_label->w,
                shoulder_keymap_value->w + arrow_w,
                progress_label->w,
                progress_value->w + arrow_w
            };
            content_w = *std::max_element(widths.begin(), widths.end());
        }

        int num_menu_items = 5;
        Uint16 text_padding = 5;
        Uint16 max_content_h = SCREEN_HEIGHT - DIALOG_BORDER_WIDTH * 2;
        Uint16 line_height = theme_label->h + theme_value->h;
        int max_lines = std::max(1, (max_content_h + text_padding) / (line_height + text_padding));
        int num_lines_shown = std::min(num_menu_items, max_lines);
        {
            if (max_lines >= num_menu_items)
            {
                scroll_position = 0;
            }
            else if (line_selected < scroll_position)
            {
                // scroll up
                scroll_position = line_selected;
            }
            else if (line_selected >= scroll_position + max_lines - 1)
            {
                // scroll down
                scroll_position = line_selected + 1 - max_lines;
            }
        }

        Uint16 content_h = num_lines_shown * line_height + (num_lines_shown - 1) * text_padding;
        Sint16 content_y = SCREEN_HEIGHT / 2 - content_h / 2;

        draw_modal_border(
            content_w,
            content_h,
            theme,
            dest_surface
        );

        // draw text
        {
            SDL_Rect rect = {0, content_y, 0, 0};
            auto push_text = [&](SDL_Surface *surf, bool add_arrows = false) {
                Sint16 start = SCREEN_WIDTH / 2 - surf->w / 2;

                rect.x = start;
                SDL_BlitSurface(surf, NULL, dest_surface, &rect);

                if (add_arrows)
                {
                    SDL_Rect arrow_rect = {0, 0, 0, 0};
                    arrow_rect.y = rect.y + (surf->h - left_arrow->h) / 2;

                    arrow_rect.x = start - left_arrow->w;
                    SDL_BlitSurface(left_arrow.get(), NULL, dest_surface, &arrow_rect);

                    arrow_rect.x = start + surf->w;
                    SDL_BlitSurface(right_arrow.get(), NULL, dest_surface, &arrow_rect);
                }

                rect.y += surf->h;
            };

            auto is_line_shown = [this, num_lines_shown](uint32_t i) {
                return i >= scroll_position && i <= scroll_position + num_lines_shown - 1;
            };

            if (is_line_shown(0))
            {
                push_text(theme_label.get());
                push_text(theme_value.get(), line_selected == 0);
                rect.y += text_padding;
            }

            if (is_line_shown(1))
            {
                push_text(font_size_label.get());
                push_text(font_size_value.get(), line_selected == 1);
                rect.y += text_padding;
            }

            if (is_line_shown(2))
            {
                push_text(line_padding_label.get());
                push_text(line_padding_value.get(), line_selected == 2);
                rect.y += text_padding;
            }

            if (is_line_shown(3))
            {
                push_text(font_name_label.get());
                push_text(font_name_value.get(), line_selected == 3);
                rect.y += text_padding;
            }

            if (is_line_shown(4))
            {
                push_text(shoulder_keymap_label.get());
                push_text(shoulder_keymap_value.get(), line_selected == 4);
                rect.y += text_padding;
            }

            if (is_line_shown(5))
            {
                push_text(progress_label.get());
                push_text(progress_value.get(), line_selected == 5);
            }
        }

        needs_render = false;
        return true;
    }

    return false;
}

bool SettingsView::is_done()
{
    return _is_done;
}

bool SettingsView::is_modal()
{
    return true;
}

void SettingsView::on_change_theme(int dir)
{
    std::string theme = sys_styling.get_color_theme();
    sys_styling.set_color_theme(
        (dir < 0) ?
            get_prev_theme(theme) :
            get_next_theme(theme)
    );
}

void SettingsView::on_change_font_size(int dir)
{
    sys_styling.set_font_size(
        (dir < 0) ?
            sys_styling.get_prev_font_size() :
            sys_styling.get_next_font_size()
    );
}

void SettingsView::on_change_line_padding(int dir)
{
    sys_styling.set_line_padding(
        (dir < 0) ?
            sys_styling.get_prev_line_padding() :
            sys_styling.get_next_line_padding()
    );
}

void SettingsView::on_change_font_name(int dir)
{
    std::string font_name = sys_styling.get_font_name();
    sys_styling.set_font_name(
        (dir < 0) ?
            get_prev_font_name(font_name) :
            get_next_font_name(font_name)
    );
}

void SettingsView::on_change_shoulder_keymap(int dir)
{
    const auto &keymap = sys_styling.get_shoulder_keymap();
    sys_styling.set_shoulder_keymap(
        (dir < 0) ?
            get_prev_shoulder_keymap(keymap) :
            get_next_shoulder_keymap(keymap)
    );
}

void SettingsView::on_change_progress()
{
    token_view_styling.set_progress_reporting(
        get_next_progress_reporting(token_view_styling.get_progress_reporting())
    );
}

void SettingsView::on_keypress(SDL_GameControllerButton key)
{
    switch (key) {
        case SW_BTN_UP:
            line_selected = (line_selected + num_menu_items - 1) % num_menu_items;
            needs_render = true;
            break;
        case SW_BTN_DOWN:
            line_selected = (line_selected + 1) % num_menu_items;
            needs_render = true;
            break;
        case SW_BTN_LEFT:
        case SW_BTN_RIGHT:
            {
                int dir = (key == SW_BTN_LEFT) ? -1 : 1;
                if (line_selected == 0)
                {
                    on_change_theme(dir);
                }
                else if (line_selected == 1)
                {
                    on_change_font_size(dir);
                }
                else if (line_selected == 2)
                {
                    on_change_line_padding(dir);
                }
                else if (line_selected == 3)
                {
                    on_change_font_name(dir);
                }
                else if (line_selected == 4)
                {
                    on_change_shoulder_keymap(dir);
                }
                else
                {
                    on_change_progress();
                }
                needs_render = true;
            }
            break;
        case SW_BTN_B:
            {
                _is_done = true;
            }
            break;
        default:
            break;
    }
}

void SettingsView::terminate()
{
    _is_done = true;
}

void SettingsView::unterminate()
{
    _is_done = false;
}
