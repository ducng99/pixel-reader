# Pixel Reader

An ebook reader app for [Rocknix](https://rocknix.org). Supports epub and txt formats.

> Forked from [Pixel Reader](https://github.com/ealang/pixel-reader) for Miyoo Mini

![Screenshot](resources/demo.gif)

### Changes from upstream
- New `dark_olive` theme
- Customizable line padding
- Use SDL2
- Use SDL's game controller support directly (no gptokeyb needed)

## Customize

To override default directories, some envionment variables can be set

- `STORAGE_PATH`: A writeable directory containing `resources` dir, to include fonts and configs
- `BROWSE_PATH`: A readable directory to start ebook file browser

Build flags:
- `USE_KEYBOARD`: Build for keyboard instead of game controllers

## Development Reference

### Desktop Build

Install dependencies (Ubuntu):
```
apt install make g++ libxml2-dev libzip-dev libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
```

Build:
```
make -j
```

Find app in `build/reader`.

### Run Tests

[Install gtest](https://github.com/google/googletest/blob/main/googletest/README.md).

```
make test
```
