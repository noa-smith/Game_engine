#ifndef ANDROIDPLATFORMSERVICES_HPP
#define ANDROIDPLATFORMSERVICES_HPP

#include <iostream>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

#include "PlatformServices.hpp"
#include "SDL2/SDL.h"
#include "SDL2/SDL_system.h"

class AndroidPlatformServices : public PlatformServices {
public:
    void RequestExit() override {
        SDL_Event e {};
        e.type = SDL_QUIT;
        SDL_PushEvent(&e);
    }

    void OpenUrl(const std::string &url) override {
        (void) url;
        // TODO: implement via JNI intent on Android.
    }

    bool AssetExists(const std::string &path) override {
        (void) path;
        SDL_RWops *rw = SDL_RWFromFile(path.c_str(), "rb");
        if (!rw) return false;
        SDL_RWclose(rw);
        return true;
    }

    std::string ReadTextAsset(const std::string &path) override {
        (void) path;
        size_t size;

        void *rawData = SDL_LoadFile(path.c_str(), &size);

        if (!rawData) {
            return "";
        }
        std::string content(static_cast<char *>(rawData), size);
        SDL_free(rawData);

        return content;
    }

    std::vector<std::string> ListAssetFiles(const std::string &dir) override {
#if !defined(__ANDROID__)
        (void)dir;
        return {};
#else
        std::vector<std::string> files;

        JNIEnv *env = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());
        jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
        if (!env || !activity) {
            return files;
        }

        jclass activityClass = env->GetObjectClass(activity);
        if (!activityClass) {
            env->DeleteLocalRef(activity);
            return files;
        }

        jmethodID getAssetsMethod = env->GetMethodID(activityClass, "getAssets", "()Landroid/content/res/AssetManager;");
        if (!getAssetsMethod) {
            env->DeleteLocalRef(activityClass);
            env->DeleteLocalRef(activity);
            return files;
        }

        jobject assetManagerObj = env->CallObjectMethod(activity, getAssetsMethod);
        AAssetManager *assetManager = assetManagerObj ? AAssetManager_fromJava(env, assetManagerObj) : nullptr;
        if (!assetManager) {
            if (assetManagerObj) env->DeleteLocalRef(assetManagerObj);
            env->DeleteLocalRef(activityClass);
            env->DeleteLocalRef(activity);
            return files;
        }

        std::string assetDir = dir;
        while (!assetDir.empty() && assetDir.front() == '/') {
            assetDir.erase(0, 1);
        }
        while (!assetDir.empty() && assetDir.back() == '/') {
            assetDir.pop_back();
        }

        AAssetDir *assetDirHandle = AAssetManager_openDir(assetManager, assetDir.empty() ? nullptr : assetDir.c_str());
        if (assetDirHandle) {
            for (const char *entry = AAssetDir_getNextFileName(assetDirHandle); entry;
                 entry = AAssetDir_getNextFileName(assetDirHandle)) {
                if (assetDir.empty()) {
                    files.emplace_back(entry);
                } else {
                    files.emplace_back(assetDir + "/" + entry);
                }
            }
            AAssetDir_close(assetDirHandle);
        }

        env->DeleteLocalRef(assetManagerObj);
        env->DeleteLocalRef(activityClass);
        env->DeleteLocalRef(activity);
        return files;
#endif
    }

    void LogInfo(const std::string &msg) override { std::cout << msg << std::endl; }

    void LogError(const std::string &msg) override { std::cerr << msg << std::endl; }
};

#endif
