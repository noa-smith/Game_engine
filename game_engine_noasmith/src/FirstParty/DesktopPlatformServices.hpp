#ifndef DESKTOPPLATFORMSERVICES_HPP
#define DESKTOPPLATFORMSERVICES_HPP

#include "PlatformServices.hpp"
#include "SDL2/SDL.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

class DesktopPlatformServices : public PlatformServices {
public:
    void RequestExit() override {
        SDL_Event e{};
        e.type = SDL_QUIT;
        SDL_PushEvent(&e);
    }

    void OpenUrl(const std::string& url) override {
#if defined(_WIN32)
        std::system(("start " + url).c_str());
#elif defined(__APPLE__)
        std::system(("open " + url).c_str());
#else
        std::system(("xdg-open " + url).c_str());
#endif
    }

    bool AssetExists(const std::string& path) override {
        return std::filesystem::exists(path);
    }

    std::string ReadTextAsset(const std::string& path) override {
        std::ifstream in(path);
        if (!in.is_open()) {
            return "";
        }
        std::stringstream buffer;
        buffer << in.rdbuf();
        return buffer.str();
    }

    std::vector<std::string> ListAssetFiles(const std::string& dir) override {
        std::vector<std::string> files;
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            return files;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            files.push_back(entry.path().string());
        }
        return files;
    }

    void LogInfo(const std::string& msg) override {
        std::cout << msg << std::endl;
    }

    void LogError(const std::string& msg) override {
        std::cerr << msg << std::endl;
    }
};

#endif
