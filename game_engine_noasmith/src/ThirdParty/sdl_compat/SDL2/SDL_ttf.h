#pragma once

#if __has_include(<SDL_ttf.h>)
#include <SDL_ttf.h>
#elif __has_include("SDL_ttf.h")
#include "SDL_ttf.h"
#else
#error "SDL_ttf.h not found. Install SDL2_ttf or enable bundled SDL headers."
#endif
