#ifndef APPLICATION_HPP
#define APPLICATION_HPP
//
//  Application.hpp
//  game_engine
//
//  Created by Noah Smith on 3/9/26.
//
#include <string>
#include <iostream>
#include <thread>
#include <cstdlib>

#include "Helper.h"
#include "PlatformContext.hpp"

inline static void ApplicationQuit(){
    auto* platform = GetPlatformServices();
    if (platform) {
        platform->LogInfo("ApplicationQuit requested");
        platform->RequestExit();
        return;
    }
    exit(0);
};

inline static void ApplicationSleep(int milliseconds){
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
};
inline static int ApplicationGetFrame(){
    return Helper::GetFrameNumber();
};
inline static void ApplicationOpenUrl(const std::string &url){
    auto* platform = GetPlatformServices();
    if (platform) {
        platform->OpenUrl(url);
        return;
    }
    std::string command = "";
#if defined(_WIN32)
    command = "start " + url;
#elif defined(__APPLE__)
    command = "open " + url;
#elif defined(__linux__)
    command = "xdg-open " + url;
#else
    #error "Unsupported OS for URL, could not open url
#endif
std::system(command.c_str());
};
#endif
