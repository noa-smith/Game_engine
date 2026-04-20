#pragma once

#if __has_include(<SDL_image.h>)
#include <SDL_image.h>
#elif __has_include("SDL_image.h")
#include "SDL_image.h"
#else
#error "SDL_image.h not found. Install SDL2_image or enable bundled SDL headers."
#endif
