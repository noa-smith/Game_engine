//
//  Actor.hpp
//  game_engine
//
//  Created by Noah Smith on 1/31/26.
//
#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <cstdint>
#include <string>
#include "EngineUtil.h"
#include "ComponentDB.hpp"
//#include "Scene.hpp"
#include <memory>
static int AddFunctCount = 0;
struct Actor {
    uint32_t id = 0;
    std::string name = "";
    bool destroyed = false;
    bool dont_destroy = false;
    std::map<std::string, std::shared_ptr<luabridge::LuaRef>> components;
    //std::vector<std::string> components_added_this_frame;
    std::map<std::string, std::shared_ptr<luabridge::LuaRef>> components_added_this_frame;
    std::vector<std::string> components_to_remove;
    ComponentDB *component_db;
    void AddLuaComponent(const std::string &component_id, const std::string &component_type, ComponentDB *component_db);
    void AddDebug(ComponentDB *component_db);
    void ActorFunctions(ComponentDB *component_db);
    void AddApplication();
    RigidBody2D *rb;
    std::string GetName();
    uint32_t GetID();
    void AddKey(const std::string& component_id);
    void AddKey(const std::string& component_id, bool enabled);
    void Override(const std::string& compnent_id, const std::string& key, const std::string& value);
    void Override(const std::string& component_id, const std::string& key, const float& value);
    void Override(const std::string& component_id, const std::string& key, const bool& value);
    void Override(const std::string& component_id, const std::string& key, const int& value);
    void InjectActorReference(std::shared_ptr<luabridge::LuaRef> comp_ref);
    luabridge::LuaRef GetComponentByKey(const std::string &key);
    luabridge::LuaRef GetComponent(const std::string &type);
    luabridge::LuaRef GetComponents(const std::string &type);
    luabridge::LuaRef AddComponent(const std::string &type);
    void RemoveComponent(luabridge::LuaRef comp);
    void ExecuteRemovals();
    void MoveAddedComponents();
    virtual ~Actor() = default;
    Actor() = default;
    Actor(uint32_t id, std::string name, ComponentDB *component_db_in)
        : id(id)
        , name(std::move(name))
        , component_db(component_db_in){}
};

struct CompareById {
    bool operator()(const Actor &a, const Actor &b) const { return a.id < b.id; }
    bool operator()(const Actor *a, const Actor *b) const {
        if (a == nullptr || b == nullptr) {
            return b != nullptr;
        }
        return a->id < b->id;
    }
};
// struct RenderOrder {
//     bool operator()(const Actor &a, const Actor &b) const {
//         int render_order_num_a;
//         int render_order_num_b;
//         if (a.render_order) {
//             render_order_num_a = *a.render_order;
//         } else {
//             render_order_num_a = a.transform_position.y;
//         }
//         if (b.render_order) {
//             render_order_num_b = *b.render_order;
//         } else {
//             render_order_num_b = b.transform_position.y;
//         }
//         if (render_order_num_a != render_order_num_b) {
//             return render_order_num_a < render_order_num_b;
//         } else if (a.transform_position.y != b.transform_position.y) {
//             return a.transform_position.y < b.transform_position.y;
//         } else {
//             return a.id < b.id;
//         }
//     }
//     bool operator()(const Actor *a, const Actor *b) const {
//         if (a == nullptr || b == nullptr) {
//             return b != nullptr;
//         }
//         return (*this)(*a, *b);  // pointer overload that just uses the reference version above
//     }
// };
#endif

