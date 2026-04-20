// PlatformServices.hpp
#ifndef PLATFORMSERVICES_HPP
#define PLATFORMSERVICES_HPP
#include <string>
#include <vector>

class PlatformServices {
public:
    virtual ~PlatformServices() = default;

    virtual void RequestExit() = 0;
    virtual void OpenUrl(const std::string& url) = 0;

    virtual bool AssetExists(const std::string& path) = 0;
    virtual std::string ReadTextAsset(const std::string& path) = 0;
    virtual std::vector<std::string> ListAssetFiles(const std::string& dir) = 0;

    virtual void LogInfo(const std::string& msg) = 0;
    virtual void LogError(const std::string& msg) = 0;
};
#endif

