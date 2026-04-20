//
//  EngineUtil.h
//  game_engine
//
//  Created by Noah Smith on 1/30/26.
//
#ifndef EngineUtil_h
#define EngineUtil_h

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>


#include "box2d/box2d.h"
#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "SDL2/SDL_ttf.h"
#include "Input.hpp"
#include "Application.hpp"
#include "PlatformContext.hpp"

enum class Horizontal {
    None,
    East,
    West
};

enum class Vertical {
    None,
    North,
    South
};

const std::unordered_map<std::string, SDL_Scancode> __keycode_to_scancode = {
    // Directional (arrow) Keys
    {"up", SDL_SCANCODE_UP},
    {"down", SDL_SCANCODE_DOWN},
    {"right", SDL_SCANCODE_RIGHT},
    {"left", SDL_SCANCODE_LEFT},

    // Misc Keys
    {"escape", SDL_SCANCODE_ESCAPE},

    // Modifier Keys
    {"lshift", SDL_SCANCODE_LSHIFT},
    {"rshift", SDL_SCANCODE_RSHIFT},
    {"lctrl", SDL_SCANCODE_LCTRL},
    {"rctrl", SDL_SCANCODE_RCTRL},
    {"lalt", SDL_SCANCODE_LALT},
    {"ralt", SDL_SCANCODE_RALT},

    // Editing Keys
    {"tab", SDL_SCANCODE_TAB},
    {"return", SDL_SCANCODE_RETURN},
    {"enter", SDL_SCANCODE_RETURN},
    {"backspace", SDL_SCANCODE_BACKSPACE},
    {"delete", SDL_SCANCODE_DELETE},
    {"insert", SDL_SCANCODE_INSERT},

    // Character Keys
    {"space", SDL_SCANCODE_SPACE},
    {"a", SDL_SCANCODE_A},
    {"b", SDL_SCANCODE_B},
    {"c", SDL_SCANCODE_C},
    {"d", SDL_SCANCODE_D},
    {"e", SDL_SCANCODE_E},
    {"f", SDL_SCANCODE_F},
    {"g", SDL_SCANCODE_G},
    {"h", SDL_SCANCODE_H},
    {"i", SDL_SCANCODE_I},
    {"j", SDL_SCANCODE_J},
    {"k", SDL_SCANCODE_K},
    {"l", SDL_SCANCODE_L},
    {"m", SDL_SCANCODE_M},
    {"n", SDL_SCANCODE_N},
    {"o", SDL_SCANCODE_O},
    {"p", SDL_SCANCODE_P},
    {"q", SDL_SCANCODE_Q},
    {"r", SDL_SCANCODE_R},
    {"s", SDL_SCANCODE_S},
    {"t", SDL_SCANCODE_T},
    {"u", SDL_SCANCODE_U},
    {"v", SDL_SCANCODE_V},
    {"w", SDL_SCANCODE_W},
    {"x", SDL_SCANCODE_X},
    {"y", SDL_SCANCODE_Y},
    {"z", SDL_SCANCODE_Z},
    {"0", SDL_SCANCODE_0},
    {"1", SDL_SCANCODE_1},
    {"2", SDL_SCANCODE_2},
    {"3", SDL_SCANCODE_3},
    {"4", SDL_SCANCODE_4},
    {"5", SDL_SCANCODE_5},
    {"6", SDL_SCANCODE_6},
    {"7", SDL_SCANCODE_7},
    {"8", SDL_SCANCODE_8},
    {"9", SDL_SCANCODE_9},
    {"/", SDL_SCANCODE_SLASH},
    {";", SDL_SCANCODE_SEMICOLON},
    {"=", SDL_SCANCODE_EQUALS},
    {"-", SDL_SCANCODE_MINUS},
    {".", SDL_SCANCODE_PERIOD},
    {",", SDL_SCANCODE_COMMA},
    {"[", SDL_SCANCODE_LEFTBRACKET},
    {"]", SDL_SCANCODE_RIGHTBRACKET},
    {"\\", SDL_SCANCODE_BACKSLASH},
    {"'", SDL_SCANCODE_APOSTROPHE}
};
struct ActorDir {
    Horizontal x;
    Vertical y;
};

enum class DialogueCmd {
    HealthDown, ScoreUp, YouWin, YouLose, None, ProceedTo,
};
enum class GameState {
    Intro, Scene, End, Quit
};

static const glm::vec2 ZERO = glm::vec2 (0.0f, 0.0f);

namespace fs = std::filesystem;

enum class file_type { Game, Rendering, Scene, ActorTemplate };

inline static void PlatformInfoAndExit(const std::string& msg) {
    auto* platform = GetPlatformServices();
    if (platform) {
        platform->LogInfo(msg);
        platform->RequestExit();
        return;
    }
    std::cout << msg << std::endl;
    exit(0);
}

inline std::string ReadTextAssetOrFail(const std::string& path) {
    auto* p = GetPlatformServices();
    std::string s = p ? p->ReadTextAsset(path) : "";
    if (s.empty()) PlatformInfoAndExit("error: missing asset " + path);
    return s;
}

inline std::string ReadTextAsset(const std::string &path) {
    auto *platform = GetPlatformServices();
    if (platform) {
        return platform->ReadTextAsset(path);
    }
    std::ifstream in(path);
    if (!in.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

static void ParseJsonText(const std::string &path, const std::string &json_text, rapidjson::Document &out_document) {
    out_document.Parse(json_text.c_str());
    if (out_document.HasParseError()) {
        PlatformInfoAndExit("error parsing json at [" + path + "]");
    }
}

inline std::string ResolveResourcePath(file_type type_req, std::optional<std::string> file_name = std::nullopt) {
    const std::string documents_dir = "resources";
    std::string file_path;
    switch (type_req) {
    case file_type::Game:
        file_path = documents_dir + "/game.config";
        break;
    case file_type::Rendering:
        file_path = documents_dir + "/rendering.config";
        break;
    case file_type::Scene:
        if (!file_name.has_value()) {
            PlatformInfoAndExit("error: scene name missing");
            return "";
        }
        file_path = documents_dir + "/scenes/" + *file_name + ".scene";
        break;
    case file_type::ActorTemplate:
        if (!file_name.has_value()) {
            PlatformInfoAndExit("error: template name missing");
            return "";
        }
        file_path = documents_dir + "/actor_templates/" + *file_name + ".template";
        break;
    }
    return file_path;
}
inline bool LoadResourcesImpl(rapidjson::Document &doc, file_type type_req,
                              std::optional<std::string> file_name = std::nullopt) {
    const std::string path = ResolveResourcePath(type_req, file_name);
    if (path.empty()) {
        return false;
    }

    const std::string content = ReadTextAsset(path);
    if (content.empty()) {
        if (type_req == file_type::Rendering) {
            return false;
        }
        if (type_req == file_type::Scene && file_name.has_value()) {
            PlatformInfoAndExit("error: scene " + *file_name + " is missing");
            return false;
        }
        if (type_req == file_type::ActorTemplate && file_name.has_value()) {
            PlatformInfoAndExit("error: template " + *file_name + " is missing");
            return false;
        }
        PlatformInfoAndExit("error: missing asset " + path);
        return false;
    }

    ParseJsonText(path, content, doc);
    return true;
}
inline bool LoadResources(rapidjson::Document &doc, file_type type_req) {
    return LoadResourcesImpl(doc, type_req);
}
inline bool LoadResources(rapidjson::Document &doc, file_type type_req, const std::string &file_name) {
    return LoadResourcesImpl(doc, type_req, file_name);
}
static uint64_t CreateCoordKey(int x, int y) {
    uint32_t casted_x = static_cast<uint32_t>(x);
    uint32_t casted_y = static_cast<uint32_t>(y);
    uint64_t key = casted_x;
    key = key << 32;
    key = key | static_cast<uint64_t>(casted_y);
    return key;
}
static std::string ObtainWordAfterPhrase(std::string& input, std::string& phrase){
    size_t pos = input.find(phrase);
    if(pos == std::string::npos) return "";
    pos += phrase.length();
    
    while(pos < input.size() && std::isspace(input[pos])){
        ++pos;
    }
    if(pos == input.size()) return "";
    size_t end_pos = pos;
    while(end_pos < input.size() && !std::isspace(input[end_pos])){
        ++end_pos;
    }
    return input.substr(pos, end_pos - pos);
}
struct LuaComponentInstance {
    std::string id;
    std::string type;
    int luaRef;
};

inline static void DebugLog(const std::string & message){
    auto* platform = GetPlatformServices();
    if (platform) {
        platform->LogInfo(message);
        return;
    }
    std::cout << message << std::endl;
};

inline static void ReportError(const std::string &actor_name, const luabridge::LuaException &e){
    std::string e_message = e.what();
    std::replace(e_message.begin(), e_message.end(), '\\','/');
    std::string msg = actor_name + " : " + e_message;
    auto* platform = GetPlatformServices();
    if (platform) {
        platform->LogInfo(msg);
        return;
    }
    std::cout << "\033[31m" << msg << "\033[0m" << std::endl;
}
inline static float VectorDistanceApi(const b2Vec2 a, const b2Vec2 b){
    return b2Distance(a, b);
}
inline static float DotApi(const b2Vec2& a, const b2Vec2& b){
    return b2Dot(a, b);
}
#endif
