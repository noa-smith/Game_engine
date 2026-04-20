//
//  Scene.cpp
//  game_engine
//
//  Created by Noah Smith on 1/31/26.
//
#include "Scene.hpp"
#include <cstring>

Scene* Scene::current = nullptr;

void Scene::StartLua() {
    current = this;
    lua_state = luaL_newstate();
    component_data_base = ComponentDB(lua_state);
    component_data_base.SetLuaState(lua_state);
    luaL_openlibs(lua_state);
    SceneApi();
}
lua_State *Scene::GetLuaState(){
    return lua_state;
}
Actor* Scene::FindActor(const std::string &name){
    if (!current) return nullptr;
    for (auto &matching : current->actors) {
        if(!matching) continue;
        if(matching->destroyed) continue;
        if (matching->name == name) {
            return matching.get();
        }
    }
    for (auto &matching : current->instantiated_actors){
        if(!matching) continue;
        if(matching->destroyed) continue;
        if (matching->name == name) {
            return matching.get();
        }
    }
    return luabridge::LuaRef(current->component_data_base.lua_state);
}
void Scene::SceneApi(){
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Scene")
        .addFunction("Load",&Scene::LoadNewSceneApi )
        .addFunction("GetCurrent", &Scene::GetCurrentSceneApi)
        .addFunction("DontDestroy", &Scene::DontDestroyActorApi)
        .endNamespace();
}
void Scene::LoadNewSceneApi(const std::string& scene_name) {
    current->current_scene_name = scene_name;
    current->load_new_scene = true;
}
std::string Scene::GetCurrentSceneApi(){
    return current->current_scene_name;
}
void Scene::DontDestroyActorApi(Actor* actor_) {
    if (actor_->id >= 0 && static_cast<size_t>(actor_->id) < current->actors.size() && current->actors[actor_->id]) {
        auto& keep = current->actors[actor_->id];
        if (keep) {
            keep->dont_destroy = true;
        }
    }
    else {
        for(auto &actor : current->instantiated_actors){
            if(actor->id == actor_->id){
                actor->dont_destroy = true;
            }
        }
    }
}
Actor* Scene::InstantiateActor(const std::string &template_name){
    Template template_;
    std::string name = "";
    std::string type = template_name;
    auto new_actor = std::make_unique<Actor>(current->id, name, &current->component_data_base);
    template_.LoadTemplateToActor(&current->component_data_base, type, new_actor.get());
    new_actor->AddApplication();
    new_actor->AddDebug(&current->component_data_base);
    new_actor->ActorFunctions(&current->component_data_base);
    for(auto comp : new_actor->components){
        new_actor->AddKey(comp.first, false);
        new_actor->InjectActorReference(comp.second);
    }
    current->instantiated_actors.push_back(std::move(new_actor));
    current->id++;
    auto pushed_actor = current->instantiated_actors.back().get();
    return pushed_actor;
}
void Scene::DestroyActor(const Actor& actor_in) {
    if (!current) return;

    auto disable_and_mark = [](Actor* actor) {
        if (!actor || actor->destroyed) return;

        for (auto& [name, comp] : actor->components) {
            (*comp)["enabled"] = false;
        }
        actor->destroyed = true;
    };
    for (auto& actor : current->actors) {
        if (actor && actor->id == actor_in.id) {
            disable_and_mark(actor.get());
            return;
        }
    }
    for (auto& actor : current->instantiated_actors) {
        if (actor && actor->id == actor_in.id) {
            disable_and_mark(actor.get());
            return;
        }
    }
}
void Scene::DestroyActorQueue() {
    if (!current) return;
    auto& actors = current->actors;
    actors.erase(
        std::remove_if(
            actors.begin(),
            actors.end(),
            [](const std::unique_ptr<Actor>& actor) {
                return actor && actor->destroyed;
            }
        ),
        actors.end()
    );
    auto& pending = current->instantiated_actors;
    pending.erase(
        std::remove_if(
            pending.begin(),
            pending.end(),
            [](const std::unique_ptr<Actor>& actor) {
                return actor && actor->destroyed;
            }
        ),
        pending.end()
    );
}
luabridge::LuaRef Scene::FindAllActors(const std::string &name){
    luabridge::LuaRef actors_with_name = luabridge::newTable(current->component_data_base.lua_state);
    int index = 0;
    for(auto &matching : current->actors){
        if(!matching) continue;
        if(matching->destroyed) continue;
        if(matching->name == name){
            actors_with_name[++index] = matching.get();
        }
    }
    for (auto &matching : current->instantiated_actors){
        if(!matching) continue;
        if(matching->destroyed) continue;
        if (matching->name == name) {
            actors_with_name[++index] = matching.get();
        }
    }
    if(index == 0){
        return luabridge::LuaRef(current->component_data_base.lua_state);
    }
    else {
        return actors_with_name;
    }
}
void Scene::StartInstantiated() {
    auto& pending = current->instantiated_actors;
    auto& actors = current->actors;
    actors.reserve(actors.size() + pending.size());
    for (auto& p : pending) {
        if (!p || p->destroyed) {
            continue;
        }
        Actor* actor = p.get();
        for (auto& [name, comp] : actor->components) {
            (*comp)["enabled"] = true;
        }
        actors.push_back(std::move(p));
    }
    pending.clear();
}
void Scene::ActorObject(){
    luabridge::getGlobalNamespace(component_data_base.lua_state)
        .beginNamespace("Actor")
        .addFunction("Find", &Scene::FindActor)
        .addFunction("FindAll", &Scene::FindAllActors)
        .addFunction("Instantiate", &Scene::InstantiateActor)
        .addFunction("Destroy", &Scene::DestroyActor)
        .endNamespace();
}

void Scene::LoadScene(Renderer &engine_renderer) {
    TTF_Init();
    rapidjson::Document renderingConfig;
    game_renderer_ptr = &engine_renderer;
    bool game_config_loaded = LoadResources(gameConfig, file_type::Game);  // checks and loads gameconfig
    bool rendering_config_loaded
      = LoadResources(renderingConfig, file_type::Rendering);  // load new camera dimensions if present
    if (rendering_config_loaded) {
        int red = 255;
        int blue = 255;
        int green = 255;
        if (renderingConfig.HasMember("x_resolution")) {
            CAMERA_WIDTH = renderingConfig["x_resolution"].GetInt();
        }
        if (renderingConfig.HasMember("y_resolution")) {
            CAMERA_HEIGHT = renderingConfig["y_resolution"].GetInt();
        }
        if (renderingConfig.HasMember("clear_color_r")) {
            red = renderingConfig["clear_color_r"].GetInt();
        }
        if (renderingConfig.HasMember("clear_color_b")) {
            blue = renderingConfig["clear_color_b"].GetInt();
        }
        if (renderingConfig.HasMember("clear_color_g")) {
            green = renderingConfig["clear_color_g"].GetInt();
        }
        engine_renderer.SetClearColor(red, blue, green);
    }
    if (!game_config_loaded) {
        PlatformInfoAndExit("error: game.config failed to load");
        return;
    }
    if (gameConfig.HasMember("game_title")) {
        SetGameTitle(gameConfig["game_title"].GetString());
    }
    engine_renderer.SetWindowParameters(GetGameTitle(), CAMERA_WIDTH, CAMERA_HEIGHT);
    engine_renderer.StartRenderer();
    if (gameConfig.HasMember("font")) {
        std::string file_name = gameConfig["font"].GetString();
        fs::path font_path = fs::path("resources/fonts") / (file_name + ".ttf");
        SDL_RWops* rw = SDL_RWFromFile(font_path.string().c_str(), "rb");
        game_font = rw ? TTF_OpenFontRW(rw, 1, 16) : nullptr;
        if (!game_font) {
            PlatformInfoAndExit("error: font " + font_path.filename().stem().string() + " missing");
            return;
        }
    }
    if (gameConfig.HasMember("initial_scene")) {
        std::string initial_scene = gameConfig["initial_scene"].GetString();
        if (initial_scene.empty()) {
            PlatformInfoAndExit("error: initial_scene unspecified");
            return;
        }
        if(!current->world_created){
            current->world_created = true;
            current->world = new b2World(current->gravity);
        }
        Add_Actors(initial_scene);
    } else {
        PlatformInfoAndExit("error: initial_scene unspecified");
        return;
    }
}
void Scene::LoadNewScene() {
    load_new_scene = false;
    auto& actors = current->actors;
    actors.erase(
        std::remove_if(
            actors.begin(),
            actors.end(),
            [](const std::unique_ptr<Actor>& actor) {
                return actor && !actor->dont_destroy;
            }
        ),
        actors.end()
    );
    Add_Actors(current_scene_name);
}
std::string &Scene::GetGameTitle() {
    return game_window_title;
}
void Scene::SetGameTitle(std::string str_in) {
    game_window_title = std::move(str_in);
}
void Scene::Add_Actors(const std::string &scene_in) {
    //uint32_t id = 0;
    Template template_;
    rapidjson::Document doc;
    bool scene_loaded = LoadResources(doc, file_type::Scene, scene_in);
    if (!scene_loaded) {
        PlatformInfoAndExit("error: scene load error");
        return;
    }
    current_scene_name = scene_in;
    const auto &actors_json = doc["actors"];
    actors.reserve(doc["actors"].Size()*32);
    instantiated_actors.reserve(doc["actors"].Size()*32);
    game_renderer_ptr->ReserveActorSpace(doc["actors"].Size()*32);
    std::string components_dir = "resources/component_types/";
    for (const auto &actor_entry : actors_json.GetArray()) {
        std::string name;

        if (actor_entry.HasMember("name")) {
            name = actor_entry["name"].GetString();
        }
        
        auto new_actor = std::make_unique<Actor>(id, name, &component_data_base);
        if(actor_entry.HasMember("template")){
            std::string template_req = actor_entry["template"].GetString();
            template_.LoadTemplateToActor(&component_data_base, template_req, new_actor.get());
        }
        if (actor_entry.HasMember("components") && actor_entry["components"].IsObject()) {
            const auto &components_json = actor_entry["components"];

            for (auto itr = components_json.MemberBegin(); itr != components_json.MemberEnd(); ++itr) {
                std::string component_id = itr->name.GetString();
                const auto &component_obj = itr->value;
                if (!component_obj.IsObject()) continue;
                if(new_actor->components.find(component_id) != new_actor->components.end()){
                    for(auto comp_override = component_obj.MemberBegin(); comp_override != component_obj.MemberEnd(); ++comp_override){
                        if (std::strcmp(comp_override->name.GetString(), "type") == 0) continue;
                        if(comp_override->value.IsString()){
                            std::string message = comp_override->value.GetString();
                            new_actor->Override(component_id, comp_override->name.GetString(), message);
                        }
                        else if(comp_override->value.IsFloat()){
                            new_actor->Override(component_id, comp_override->name.GetString(), comp_override->value.GetFloat());
                        }
                        else if(comp_override->value.IsInt()){
                            new_actor->Override(component_id, comp_override->name.GetString(), comp_override->value.GetInt());
                        }
                        else if(comp_override->value.IsBool()){
                            new_actor->Override(component_id, comp_override->name.GetString(), comp_override->value.GetBool());
                        }
                        else{
                            PlatformInfoAndExit("Unsupported Type at:" + component_id + ", " + comp_override->name.GetString());
                            return;
                        }
                    }
                    continue;
                }
                if (!component_obj.HasMember("type") || !component_obj["type"].IsString()) continue;
                std::string component_type = component_obj["type"].GetString();

                component_data_base.LoadComponentType(component_type, components_dir);
                new_actor->AddLuaComponent(component_id, component_type, &component_data_base);
                new_actor->AddKey(static_cast<const std::string>(component_id));
                for(auto comp_override = component_obj.MemberBegin(); comp_override != component_obj.MemberEnd(); ++comp_override){
                    if (std::strcmp(comp_override->name.GetString(), "type") == 0) continue;
                    if(comp_override->value.IsString()){
                        std::string message = comp_override->value.GetString();
                        new_actor->Override(component_id, comp_override->name.GetString(), message);
                    }
                    else if(comp_override->value.IsFloat()){
                        new_actor->Override(component_id, comp_override->name.GetString(), comp_override->value.GetFloat());
                    }
                    else if(comp_override->value.IsInt()){
                        new_actor->Override(component_id, comp_override->name.GetString(), comp_override->value.GetInt());
                    }
                    else if(comp_override->value.IsBool()){
                        new_actor->Override(component_id, comp_override->name.GetString(), comp_override->value.GetBool());
                    }
                    else{
                        PlatformInfoAndExit("Unsupported Type at:" + component_id + ", " + comp_override->name.GetString());
                        return;
                    }
                    
                }
            }
        }
        new_actor->AddApplication();
        new_actor->AddDebug(&component_data_base);
        new_actor->ActorFunctions(&component_data_base);
        ActorObject();
        for(auto comp : new_actor->components){
            new_actor->InjectActorReference(comp.second);
        }
        if (id < actors.size()) {
            actors[id] = std::move(new_actor);
        }
        else {
            actors.push_back(std::move(new_actor));
        }
        id++;
    }
}
