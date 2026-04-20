#pragma once

#if __has_include(<SDL_mixer.h>)
#include <SDL_mixer.h>
#elif __has_include("SDL_mixer.h")
#include "SDL_mixer.h"
#else
#error "SDL_mixer.h not found. Install SDL2_mixer or enable bundled SDL headers."
#endif
