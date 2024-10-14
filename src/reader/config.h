#ifndef CONFIG_H_
#define CONFIG_H_

#define TARGET_FPS 20

#define IDLE_SAVE_TIME_SEC 60

#ifndef DEFAULT_STORAGE
#define DEFAULT_STORAGE "/storage/.config/pixelreader/"
#endif

#define FONT_DIR            DEFAULT_STORAGE "resources/fonts"
#define DEFAULT_FONT_NAME   DEFAULT_STORAGE "resources/fonts/DejaVuSans.ttf"
#define SYSTEM_FONT         DEFAULT_STORAGE "resources/fonts/DejaVuSansMono.ttf"

#define MIN_FONT_SIZE      18
#define MAX_FONT_SIZE      40
#define DEFAULT_FONT_SIZE  26
#define FONT_SIZE_STEP     2

#define DIALOG_PADDING       25
#define DIALOG_BORDER_WIDTH  3

#define DEFAULT_COLOR_THEME "night_contrast"

#define CONFIG_FILE_PATH "reader.cfg"
#define FALLBACK_STORE_PATH DEFAULT_STORAGE ".pixel_reader_store"

#define DEFAULT_BROWSE_PATH "/roms/media/books/"
#define EXTRA_FONTS_LIST {}

#define DEFAULT_SHOW_PROGRESS true
#define DEFAULT_SHOULDER_KEYMAP "LR"

#define DEFAULT_PROGRESS_REPORTING ProgressReporting::GLOBAL_PERCENT

#endif
