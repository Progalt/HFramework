#pragma once


#ifdef _WIN32
#define HF_PLATFORM_WINDOWS
#endif

#ifdef __linux__
#define HF_PLATFORM_LINUX
#endif

#ifdef APPLE
#define HF_PLATFORM_APPLE
#endif

#ifdef __EMSCRIPTEN__
#define HF_PLATFORM_WEB
#endif

#if defined(HF_PLATFORM_WINDOWS) || defined(HF_PLATFORM_LINUX)
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#endif

#ifdef HF_PLATFORM_WEB

#endif