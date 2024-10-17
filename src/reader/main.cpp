#include "./config.h"
#include "./font_catalog.h"
#include "./settings_store.h"
#include "./shoulder_keymap.h"
#include "./state_store.h"
#include "./system_styling.h"
#include "./color_theme_def.h"
#include "./view_stack.h"
#include "./views/file_selector.h"
#include "./views/reader_bootstrap_view.h"
#include "./views/settings_view.h"
#include "./views/token_view/token_view_styling.h"
#include "filetypes/open_doc.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/fps_limiter.h"
#include "util/held_key_tracker.h"
#include "util/key_value_file.h"
#include "util/math.h"
#include "util/sdl_font_cache.h"
#include "util/task_queue.h"
#include "util/timer.h"

#include <libxml/parser.h>
#include <SDL2/SDL.h>

#include <csignal>
#include <iostream>

namespace
{

void initialize_views(
    ViewStack &view_stack,
    StateStore &state_store,
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    TaskQueue &task_queue,
    std::optional<std::filesystem::path> requested_book_path
)
{
    auto load_book = [&view_stack, &state_store, &sys_styling, &token_view_styling, &task_queue](std::filesystem::path path) {
        if (!std::filesystem::exists(path))
        {
            std::cerr << path << " does not exist" << std::endl;
            return;
        }
        if (!file_type_is_supported(path))
        {
            std::cerr << path << " filetype is not supported" << std::endl;
            return;
        }

        view_stack.push(
            std::make_shared<ReaderBootstrapView>(
                path,
                sys_styling,
                token_view_styling,
                view_stack,
                state_store,
                [&task_queue](task_func task){ task_queue.submit(task); }
            )
        );
    };

    if (requested_book_path)
    {
        load_book(*requested_book_path);
    }
    else
    {
        auto browse_path = state_store.get_current_browse_path().value_or(DEFAULT_BROWSE_PATH);
        std::shared_ptr<FileSelector> fs = std::make_shared<FileSelector>(
            browse_path,
            sys_styling
        );

        fs->set_on_file_selected(load_book);
        fs->set_on_file_focus([&state_store](std::string path) {
            state_store.set_current_browse_path(path);
        });
        fs->set_on_view_focus([&state_store]() {
            state_store.remove_current_book_path();
        });

        view_stack.push(fs);

        if (state_store.get_current_book_path())
        {
            load_book(state_store.get_current_book_path().value());
        }
    }
}

class SystemKeyChordTracker
{
    bool _select_held = false;
    bool _exit_requested = false;

public:

    // Report keypress event. Return filtered key code.
    SW_BTN_TYPE on_keypress(SW_BTN_TYPE key)
    {
        // Block any other keys while special key is held
        SW_BTN_TYPE filtered_key = (_select_held) ? SW_BTN_UNKNOWN : key;

        if (key == SW_BTN_SELECT)
        {
            _select_held = true;
        }
        else if (key == SW_BTN_START)
        {
            if (_select_held) {
                _exit_requested = true;
            }
        }

        return filtered_key;
    }

    // Report keyrelease event.
    void on_keyrelease(SW_BTN_TYPE key)
    {
        if (key == SW_BTN_SELECT)
        {
            _select_held = false;
        }
    }
    
    bool exit_requested() const
    {
        return _exit_requested;
    }
};

bool quit = false;

void signal_handler(int)
{
    quit = true;
}

const char *CONFIG_KEY_STORE_PATH = "store_path";

std::unordered_map<std::string, std::string> load_config_with_defaults()
{
    auto config = load_key_value(CONFIG_FILE_PATH);
    config.try_emplace(CONFIG_KEY_STORE_PATH, FALLBACK_STORE_PATH);
    return config;
}

} // namespace

int main(int argc, char **argv)
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (char* env_screen_width = SDL_getenv("SCREEN_WIDTH")) {
        int new_width = atoi(env_screen_width);
        if (100 < new_width && new_width < 4096)
            SCREEN_WIDTH = static_cast<unsigned int>(new_width);
    }

    if (char* env_screen_height = SDL_getenv("SCREEN_HEIGHT")) {
        int new_height = atoi(env_screen_height);
        if (100 < new_height && new_height < 4096)
            SCREEN_HEIGHT = static_cast<unsigned int>(new_height);
    }

    std::cout << "Screen Size: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
    
    if (const char* db_file = SDL_getenv("SDL_GAMECONTROLLERCONFIG_FILE")) {
        SDL_GameControllerAddMappingsFromFile(db_file);
        std::cout << "Load SDL game controllers mapping file: " << db_file << std::endl;
    }

    // SDL Init
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();
    atexit(SDL_Quit);
    
    std::cout << "SDL initialized" << std::endl;

    // Surfaces
    SDL_Window *sdlWindow;
    SDL_Renderer *sdlRenderer;
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdlWindow, &sdlRenderer);
    SDL_Surface *screen = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_ARGB8888);
    auto sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    set_render_surface_format(screen->format);
    
    std::cout << "SDL screen set up" << std::endl;

    auto config = load_config_with_defaults();
    StateStore state_store(config[CONFIG_KEY_STORE_PATH]);
    
    std::cout << "Loaded configs" << std::endl;

    // Preload & check fonts
    auto init_font_name = get_valid_font_name(settings_get_font_name(state_store).value_or(DEFAULT_FONT_NAME));
    auto init_font_size = bound(settings_get_font_size(state_store).value_or(DEFAULT_FONT_SIZE), MIN_FONT_SIZE, MAX_FONT_SIZE);
    auto init_line_padding = bound(settings_get_line_padding(state_store).value_or(DEFAULT_LINE_PADDING), MIN_LINE_PADDING, MAX_LINE_PADDING);
    if (
        !cached_load_font(SYSTEM_FONT, init_font_size, FontLoadErrorOpt::NoThrow) ||
        !cached_load_font(init_font_name, init_font_size, FontLoadErrorOpt::NoThrow)
    )
    {
        std::cerr << "Failed to load one or more fonts" << std::endl;
        return 1;
    }
    
    std::cout << "Loaded fonts" << std::endl;

    // System styling
    SystemStyling sys_styling(
        init_font_name,
        init_font_size,
        init_line_padding,
        get_valid_theme(settings_get_color_theme(state_store).value_or(DEFAULT_COLOR_THEME)),
        get_valid_shoulder_keymap(settings_get_shoulder_keymap(state_store).value_or(DEFAULT_SHOULDER_KEYMAP))
    );
    sys_styling.subscribe_to_changes([&state_store, &sys_styling](SystemStyling::ChangeId) {
        // Persist changes
        settings_set_color_theme(state_store, sys_styling.get_color_theme());
        settings_set_font_name(state_store, sys_styling.get_font_name());
        settings_set_font_size(state_store, sys_styling.get_font_size());
        settings_set_line_padding(state_store, sys_styling.get_line_padding());
        settings_set_shoulder_keymap(state_store, sys_styling.get_shoulder_keymap());
    });
    
    std::cout << "System stylings" << std::endl;

    // View styling
    TokenViewStyling token_view_styling(
        settings_get_show_title_bar(state_store).value_or(DEFAULT_SHOW_PROGRESS),
        settings_get_progress_reporting(state_store).value_or(DEFAULT_PROGRESS_REPORTING)
    );
    token_view_styling.subscribe_to_changes([&token_view_styling, &state_store]() {
        // Persist changes
        settings_set_show_title_bar(state_store, token_view_styling.get_show_title_bar());
        settings_set_progress_reporting(state_store, token_view_styling.get_progress_reporting());
    });
    
    std::cout << "View stylings" << std::endl;

    // Setup views
    TaskQueue task_queue;
    ViewStack view_stack;

    std::optional<std::filesystem::path> requested_book_path = (
        argc == 2 ? std::optional<std::filesystem::path>(argv[1]) : std::nullopt
    );
    initialize_views(
        view_stack,
        state_store,
        sys_styling,
        token_view_styling,
        task_queue,
        requested_book_path
    );
    quit = view_stack.is_done();

    std::shared_ptr<SettingsView> settings_view = std::make_shared<SettingsView>(
        sys_styling,
        token_view_styling,
        SYSTEM_FONT
    );
    
    std::cout << "Set up views" << std::endl;

    // Track held keys
    HeldKeyTracker held_key_tracker(
        {
            SW_BTN_UP,
            SW_BTN_DOWN,
            SW_BTN_LEFT,
            SW_BTN_RIGHT,
            SW_BTN_L1,
            SW_BTN_R1,
            SW_BTN_L2,
            SW_BTN_R2
        }
    );
    SystemKeyChordTracker chord_tracker;

    auto key_held_callback = [&view_stack](SW_BTN_TYPE key, uint32_t held_ms) {
        view_stack.on_keyheld(key, held_ms);
    };
    
    std::cout << "Track held keys" << std::endl;

    // Timing
    Timer idle_timer;
    FPSLimiter limit_fps(TARGET_FPS);
    const uint32_t avg_loop_time = 1000 / TARGET_FPS;
    
    std::cout << "Timing ran" << std::endl;

    // Initial render
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);
    
    std::cout << "Initial render" << std::endl;
    
    bool ran_user_code = true;

    while (!quit)
    {
        ran_user_code = task_queue.drain() || ran_user_code;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
#ifdef USE_KEYBOARD
                case SDL_KEYDOWN:
                    {
                        idle_timer.reset();

                        SW_BTN_TYPE key = chord_tracker.on_keypress(event.key.keysym.scancode);

                        held_key_tracker.on_keypress(key);
                        view_stack.on_keypress(key);

                        if (key == SW_BTN_POWER)
                        {
                            state_store.flush();
                        }
                        else if (key == SW_BTN_X)
                        {
                            if (view_stack.top_view() != settings_view)
                            {
                                settings_view->unterminate();
                                view_stack.push(settings_view);
                            }
                            else
                            {
                                settings_view->terminate();
                            }
                        }

                        ran_user_code = true;
                    }
                    break;
                case SDL_KEYUP:
                    chord_tracker.on_keyrelease(event.key.keysym.scancode);
                    held_key_tracker.on_keyrelease(event.key.keysym.scancode);
                    break;
#else
                case SDL_CONTROLLERDEVICEADDED:
                    SDL_GameControllerOpen(event.cdevice.which);
                    break;
                case SDL_CONTROLLERDEVICEREMOVED:
                    if (SDL_GameController* controller = SDL_GameControllerFromInstanceID(event.cdevice.which)) {
                        SDL_GameControllerClose(controller);
                    }
                    break;
                case SDL_CONTROLLERBUTTONDOWN:
                    {
                        idle_timer.reset();

                        SW_BTN_TYPE key = chord_tracker.on_keypress((SW_BTN_TYPE)event.cbutton.button);

                        held_key_tracker.on_keypress(key);
                        view_stack.on_keypress(key);

                        if (key == SW_BTN_X)
                        {
                            if (view_stack.top_view() != settings_view)
                            {
                                settings_view->unterminate();
                                view_stack.push(settings_view);
                            }
                            else
                            {
                                settings_view->terminate();
                            }
                        }

                        ran_user_code = true;
                    }
                    break;
                case SDL_CONTROLLERBUTTONUP:
                    {
                        SW_BTN_TYPE key = (SW_BTN_TYPE)event.cbutton.button;
                        chord_tracker.on_keyrelease(key);
                        held_key_tracker.on_keyrelease(key);
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.scancode == SW_BTN_POWER)
                    {
                        state_store.flush();
                    }
                    break;
#endif
                default:
                    break;
            }
        }
        
        quit = quit || chord_tracker.exit_requested();

        held_key_tracker.accumulate(avg_loop_time); // Pretend perfect loop timing for event firing consistency
        ran_user_code = held_key_tracker.for_longest_held(key_held_callback) || ran_user_code;

        if (ran_user_code)
        {
            bool force_render = view_stack.pop_completed_views();

            if (view_stack.is_done())
            {
                quit = true;
            }

            if (view_stack.render(screen, force_render))
            {
                SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
                SDL_RenderClear(sdlRenderer);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
                SDL_RenderPresent(sdlRenderer);
            }
            
            ran_user_code = false;
        }

        if (!quit)
        {
            limit_fps();
        }

        if (idle_timer.elapsed_sec() >= IDLE_SAVE_TIME_SEC)
        {
            // Make sure state is saved in case device auto-powers down. Don't seem
            // to get a signal on miyoo mini when this happens.
            state_store.flush();
            idle_timer.reset();
        }
    }

    view_stack.shutdown();
    state_store.flush();

    SDL_FreeSurface(screen);
    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyWindow(sdlWindow);
    xmlCleanupParser();
    
    return 0;
}
