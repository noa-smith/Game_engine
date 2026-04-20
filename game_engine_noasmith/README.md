# Game Engine

## SDL Dependency Strategy

This project now supports two SDL dependency modes in CMake:

1. System SDL (default, recommended)
2. Bundled SDL fallback (`SDL2/`, `SDL2_img/`, `SDL2_mix/`, `SDL2_ttf/`)

### Default: use system SDL packages

Build:

```bash
cmake -S . -B build
cmake --build build -j
```

Required libraries:

- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_ttf

Example install commands:

```bash
# macOS (Homebrew)
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf

# Ubuntu/Debian
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

### Force bundled SDL fallback

Use this when your local repo has known-good bundled SDL binaries/headers:

```bash
cmake -S . -B build -DGAMEENGINE_USE_SYSTEM_SDL=OFF
cmake --build build -j
```

### Disable bundled fallback (strict system-only)

```bash
cmake -S . -B build -DGAMEENGINE_ALLOW_BUNDLED_SDL=OFF
cmake --build build -j
```

## Makefile SDL Modes

The root `Makefile` uses the same SDL strategy:

```bash
# Default: system SDL via pkg-config
make -j

# Force bundled SDL fallback
make USE_SYSTEM_SDL=0 USE_BUNDLED_SDL=1 -j

# Strict system-only
make USE_SYSTEM_SDL=1 USE_BUNDLED_SDL=0 -j
```
