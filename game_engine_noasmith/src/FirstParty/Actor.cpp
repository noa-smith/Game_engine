//  Actor.cpp
//  game_engine
//
//  Implementation moved from header to avoid duplicate symbols.

#include "Actor.hpp"
#include "Scene.hpp"

void Actor::AddLuaComponent(const std::string &component_id, const std::string &component_type, ComponentDB *component_db) {
    luabridge::LuaRef instance = component_db->CreateInstance(component_type);
    components.emplace(component_id, std::make_shared<luabridge::LuaRef>(std::move(instance)));
}
void Actor::AddDebug(ComponentDB *component_db){
    luabridge::getGlobalNamespace(component_db->lua_state)
        .beginNamespace("Debug")
        .addFunction("Log", DebugLog)
        .endNamespace();
}
void Actor::AddApplication(){
    luabridge::getGlobalNamespace(component_db->lua_state)
        .beginNamespace("Application")
        .addFunction("Quit", ApplicationQuit)
        .addFunction("Sleep", ApplicationSleep)
        .addFunction("GetFrame", ApplicationGetFrame)
        .addFunction("OpenURL", ApplicationOpenUrl)
        .endNamespace();
}
void Actor::ActorFunctions(ComponentDB *component_db){
    luabridge::getGlobalNamespace(component_db->lua_state)
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent", &Actor::GetComponent)
        .addFunction("GetComponents", &Actor::GetComponents)
        .addFunction("AddComponent", &Actor::AddComponent)
        .addFunction("RemoveComponent", &Actor::RemoveComponent)
        .endClass();
}
void Actor::AddKey(const std::string& component_id) {
    auto component = components.find(component_id);
    if (component == components.end()){
        component = components_added_this_frame.find(component_id);
    }
    if(!component->second || component == components_added_this_frame.end()){
        return;
    }
    luabridge::LuaRef& table = *component->second;
    table["key"] = component_id;
    table["enabled"] = true;
    table["removed"] = false;
    table["OnStartRan"] = false;
}
void Actor::AddKey(const std::string &component_id, bool enabled){
    auto component = components.find(component_id);
    if (component == components.end()){
        component = components_added_this_frame.find(component_id);
    }
    if(!component->second || component == components_added_this_frame.end()){
        return;
    }
    luabridge::LuaRef& table = *component->second;
    table["key"] = component_id;
    table["enabled"] = enabled;
    table["removed"] = false;
    table["OnStartRan"] = false;
}
void Actor::Override(const std::string& component_id, const std::string& key, const std::string& value){
    auto component = components.find(component_id);
    if (component == components.end() || !component->second) return;
    luabridge::LuaRef& table = *component->second;
    std::string old_message = table[key];
    table[key] = value;
}
void Actor::Override(const std::string& component_id, const std::string& key, const float& value){
    auto component = components.find(component_id);
    if (component == components.end() || !component->second) return;
    luabridge::LuaRef& table = *component->second;
    table[key] = value;
}
void Actor::Override(const std::string& component_id, const std::string& key, const int& value){
    auto component = components.find(component_id);
    if (component == components.end() || !component->second) return;
    luabridge::LuaRef& table = *component->second;
    table[key] = value;
}
void Actor::Override(const std::string& component_id, const std::string& key, const bool& value){
    auto component = components.find(component_id);
    if (component == components.end() || !component->second) return;
    luabridge::LuaRef& table = *component->second;
    table[key] = value;
}

luabridge::LuaRef Actor::GetComponentByKey(const std::string &key){
    auto reference = components.find(key);
    if(reference != components.end() && reference->second != nullptr){
        if((*reference->second)["removed"]) return luabridge::LuaRef(component_db->lua_state);
        return *reference->second;
    }
    else {
        return luabridge::LuaRef(component_db->lua_state);
    }
}
luabridge::LuaRef Actor::GetComponent(const std::string& type) {
    for (auto& [key, comp] : components) {
        if ((*comp)["removed"]) continue;

        auto comp_type = (*comp)["type"];
        if (comp_type == type) {
            return *comp;
        }
    }

    for (auto& [key, comp] : components_added_this_frame) {
        if ((*comp)["removed"]) continue;

        auto comp_type = (*comp)["type"];
        if (comp_type == type) {
            return *comp;
        }
    }

    return luabridge::LuaRef(component_db->lua_state);
}
luabridge::LuaRef Actor::GetComponents(const std::string& type) {
    luabridge::LuaRef matching_components = luabridge::newTable(component_db->lua_state);
    int index = 0;

    for (auto& [key, comp] : components) {
        if ((*comp)["removed"]) continue;

        auto comp_type = (*comp)["type"];
        if (comp_type == type) {
            matching_components[++index] = *comp;
        }
    }

    for (auto& [key, comp] : components_added_this_frame) {
        if ((*comp)["removed"]) continue;

        auto comp_type = (*comp)["type"];
        if (comp_type == type) {
            matching_components[++index] = *comp;
        }
    }

    if (index == 0) {
        return luabridge::LuaRef(component_db->lua_state);
    }

    return matching_components;
}

luabridge::LuaRef Actor::AddComponent(const std::string &type){
    std::string key = "r" + std::to_string(AddFunctCount);
    std::string components_dir = "resources/component_types/";
    component_db->LoadComponentType(type, components_dir);
    luabridge::LuaRef instance = component_db->CreateInstance(type);
    
    components_added_this_frame.emplace(key, std::make_shared<luabridge::LuaRef>(std::move(instance)));
    AddKey(static_cast<const std::string>(key), false);
    this->InjectActorReference(components_added_this_frame.at(key));
    AddFunctCount++;
    if(type == "Rigidbody"){
        luabridge::LuaRef& comp = instance;

        luabridge::LuaRef typeRef = comp["type"];
        if (comp.isUserdata()) {
            
            try {
                RigidBody2D* rb = comp.cast<RigidBody2D*>();
                if (rb) {
                    rb->OnStart(Scene::current->world);
                }
            } catch (const luabridge::LuaException&) {
            }
        }
    }
    return *components_added_this_frame.at(key).get();
}
void Actor::RemoveComponent(luabridge::LuaRef comp){
    luabridge::LuaRef keyRef = comp["key"];
    if (!keyRef.isString()) {
        return;
    }
    const std::string key = keyRef.cast<std::string>();
    auto component_to_remove = components.find(key);
    if (component_to_remove == components.end()){
        component_to_remove = components_added_this_frame.find(key);
    }
    if(!component_to_remove->second || component_to_remove == components_added_this_frame.end()){
        return;
    }
        (*component_to_remove->second)["enabled"] = false;
        (*component_to_remove->second)["removed"] = true;
        components_to_remove.push_back(key);
}
void Actor::ExecuteRemovals(){
    for(std::string comp : components_to_remove){
        components.erase(comp);
    }
    components_to_remove.clear();
}
void Actor::InjectActorReference(std::shared_ptr<luabridge::LuaRef> comp_ref){
    luabridge::LuaRef& v = *comp_ref;
    if (v.isUserdata()) {
        RigidBody2D* rb = v.cast<RigidBody2D*>();
        rb->SetActor(this);
        this->rb = rb;
        return;
    }
    (*comp_ref)["actor"] = this;
}
uint32_t Actor::GetID(){
    return this->id;
}
std::string Actor::GetName(){
    return this->name;
}
void Actor::MoveAddedComponents() {
    if (components_added_this_frame.empty()) return;
    for (auto& [key, comp_ref] : components_added_this_frame) {
        components.emplace(key, comp_ref);
        (*components[key])["enabled"] = true;
    }
    components_added_this_frame.clear();
}
