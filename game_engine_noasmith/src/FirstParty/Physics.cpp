//
//  Physics.cpp
//  game_engine
//
//  Created by Noah Smith on 4/5/26.
//
#include "Physics.hpp"
#include "Actor.hpp"

luabridge::LuaRef Physics::Raycast(b2Vec2 pos, b2Vec2 dir, float dist){
    lua_State* L = Scene::current ? Scene::current->GetLuaState() : nullptr;
    if (L == nullptr || !Scene::current || !Scene::current->world) {
        return luabridge::LuaRef(L);
    }

    b2Vec2 n = dir;
    float len = n.Length();
    if (len <= 0.0001f) {
        return luabridge::LuaRef(L);
    }
    n *= (1.0f / len);

    b2Vec2 p1 = pos;
    b2Vec2 p2 = pos + dist * n;

    struct ClosestHitCallback : b2RayCastCallback {
        bool hit = false;
        float bestFraction = 1.0f;
        b2Vec2 point{};
        b2Vec2 normal{};
        b2Fixture* fixture = nullptr;

        float ReportFixture(b2Fixture* f, const b2Vec2& p, const b2Vec2& n, float fraction) override {
            if (fraction < bestFraction) {
                bestFraction = fraction;
                point = p;
                normal = n;
                fixture = f;
                hit = true;
            }
            return fraction;
        }
    } cb;

    Scene::current->world->RayCast(&cb, p1, p2);

    if (!cb.hit || cb.fixture == nullptr) {
        return luabridge::LuaRef(L);
    }

    Actor* actor = reinterpret_cast<Actor*>(cb.fixture->GetUserData().pointer);
    if(!actor->rb->has_trigger && !actor->rb->has_collider){
        return  luabridge::LuaRef(L);
    }
    if (actor == nullptr) {
        return luabridge::LuaRef(L);
    }

    luabridge::LuaRef result = luabridge::newTable(L);
    result["actor"] = actor;
    result["point"] = cb.point;
    result["normal"] = cb.normal;
    result["is_trigger"] = cb.fixture->IsSensor();
    return result;
}

std::vector<HitResult> Physics::RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist){
    std::vector<HitResult> results;

    if (!Scene::current || !Scene::current->world) {
        return results;
    }
    b2Vec2 n = dir;
    float len = n.Length();
    if (len <= 0.0001f) return results;
    n *= (1.0f / len);

    const b2Vec2 p1 = pos;
    const b2Vec2 p2 = pos + dist * n;

    struct AllHitsCallback : b2RayCastCallback {
        struct Entry {
            float fraction;
            HitResult hit;
        };
        std::vector<Entry> entries;

        float ReportFixture(b2Fixture* f, const b2Vec2& p, const b2Vec2& n, float fraction) override {
            if (fraction <= 0.0f) {
                return 1.0f;
            }
            Actor* actor = nullptr;
            if (f) {
                const uintptr_t ptr = f->GetUserData().pointer;
                actor = reinterpret_cast<Actor*>(ptr);
                if(!actor->rb->has_trigger && !actor->rb->has_collider){
                    return -1.0f;
                }
            }
            if (actor == nullptr) {
                return -1.0f;
            }

            Entry e;
            e.fraction = fraction;
            e.hit.actor = actor;
            e.hit.point = p;
            e.hit.normal = n;
            e.hit.is_trigger = f->IsSensor();

            entries.push_back(e);
            return 1.0f;
        }
    } cb;

    Scene::current->world->RayCast(&cb, p1, p2);
    std::sort(cb.entries.begin(), cb.entries.end(), [](const AllHitsCallback::Entry& a, const AllHitsCallback::Entry& b){
        return a.fraction < b.fraction;
    });

    results.reserve(cb.entries.size());
    for (const auto& e : cb.entries) {
        results.push_back(e.hit);
    }

    return results;
}
