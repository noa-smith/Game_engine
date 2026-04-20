//
//  Physics.hpp
//  game_engine
//
//  Created by Noah Smith on 4/5/26.
//
#ifndef PHYSICS_HPP
#define PHYSICS_HPP
#include "EngineUtil.h"
#include "b2_world_callbacks.h"
#include "Scene.hpp"
#include <vector>

struct Actor;

struct HitResult {
    Actor* actor = nullptr;
    b2Vec2 point{0.0f, 0.0f};
    b2Vec2 normal{0.0f, 0.0f};
    bool is_trigger = false;

    HitResult() = default;
    HitResult(Actor* a, const b2Vec2& p, const b2Vec2& n, bool trigger)
        : actor(a), point(p), normal(n), is_trigger(trigger) {}
};

class Physics {
public:
    static luabridge::LuaRef Raycast(b2Vec2 pos, b2Vec2 dir, float dist);
    static std::vector<HitResult> RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist);
};


#endif
