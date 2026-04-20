//
//  Scene.hpp
//  game_engine
//
//  Created by Noah Smith on 1/31/26.
//
#ifndef Scene_hpp
#define Scene_hpp
#include <string>

#include "AudioDB.hpp"
#include "Renderer.hpp"
#include "Template.hpp"
#include "ComponentDB.hpp"
#include "rapidjson/document.h"
#include "ContactListener.hpp"
//#include "EngineUtil.h"

class Scene {
public:
    //Scene() = default;
    void LoadScene(Renderer &engine_renderer);
    void LoadNewScene();
    void SetGameTitle(std::string str_in);
    void SceneApi();
    std::string &GetGameTitle();
    TTF_Font *game_font;
    rapidjson::Document gameConfig;
    std::vector<std::unique_ptr<Actor>> actors;
    std::vector<std::unique_ptr<Actor>> instantiated_actors;
    static inline std::queue<int> destroyed_actors;
    static void StartInstantiated();
    std::string game_window_title = "";
    ComponentDB component_data_base;
    Renderer *game_renderer_ptr;
    bool world_created = false;
    b2World* world;
    b2Vec2 gravity{0.0f, 9.8f};
    int CAMERA_HEIGHT = 360;
    int CAMERA_WIDTH = 640;
    void StartLua();
    static Actor* FindActor(const std::string &name);
    static luabridge::LuaRef FindAllActors(const std::string &name);
    void ActorObject();
    static Actor* InstantiateActor(const std::string &template_name);
    static void DestroyActor(const Actor &actor_to_destroy);
    void DestroyActorQueue();
    static void LoadNewSceneApi(const std::string &scene_name);
    static std::string GetCurrentSceneApi();
    static void DontDestroyActorApi(Actor* actor_);
    static inline bool load_new_scene = false;
    lua_State *GetLuaState();
    static Scene* current;
private:
    uint32_t id = 0;
    std::string current_scene_name = "";
    
    lua_State *lua_state;
    void Add_Actors(const std::string &scene_in);
};
#endif
