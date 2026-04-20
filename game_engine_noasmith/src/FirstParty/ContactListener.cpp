//
//  ContactListener.cpp
//  game_engine
//
//  Created by Noah Smith on 3/31/26.
//
#include "ContactListener.hpp"
#include "Actor.hpp"

void ContactListener::BeginContact(b2Contact *contact){
    Actor* actor_contact_a = reinterpret_cast<Actor*>(contact->GetFixtureA()->GetUserData().pointer);
    Actor* actor_contact_b = reinterpret_cast<Actor*>(contact->GetFixtureB()->GetUserData().pointer);
    b2Fixture *fixture_a = contact->GetFixtureA();
    b2Fixture *fixture_b = contact->GetFixtureB();
    b2WorldManifold world_manifold;
    contact->GetWorldManifold(&world_manifold);
    b2Vec2 contact_point = world_manifold.points[0];
    b2Vec2 mani_normal = world_manifold.normal;
    b2Vec2 rel_vel_a = fixture_a->GetBody()->GetLinearVelocity() - fixture_b->GetBody()->GetLinearVelocity();
    //b2Vec2 rel_vel_b = fixture_b->GetBody()->GetLinearVelocity() - fixture_a->GetBody()->GetLinearVelocity();
    if(actor_contact_a->rb->has_trigger && actor_contact_b->rb->has_trigger){
    //if(fixture_a->IsSensor() && fixture_b->IsSensor()){ // trigger
        for (auto& [compName, compRef] : actor_contact_a->components) {
            if(!compRef->isTable()){
                continue;
            }
            bool enabled = (*compRef)["enabled"];
            if(!enabled) continue;
            auto on_trigger_enter = (*compRef)["OnTriggerEnter"];
            if(on_trigger_enter.isFunction()){
                try{
                    collision collision_info;
                    collision_info.other = actor_contact_b;
                    collision_info.point = {-999.0f,-999.0f};
                    collision_info.relative_velocity = rel_vel_a;
                    collision_info.normal = {-999.0f,-999.0f};
                    on_trigger_enter(*compRef, collision_info);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor_contact_a->name, e);
                }
            }
        }
        for (auto& [compName, compRef] : actor_contact_b->components) {
            if(!compRef->isTable()){
                continue;
            }
            bool enabled = (*compRef)["enabled"];
            if(!enabled) continue;
            auto on_trigger_enter = (*compRef)["OnTriggerEnter"];
            if(on_trigger_enter.isFunction()){
                try{
                    collision collision_info;
                    collision_info.other = actor_contact_a;
                    collision_info.point = {-999.0f,-999.0f};
                    collision_info.relative_velocity = rel_vel_a;
                    collision_info.normal = {-999.0f,-999.0f};
                    on_trigger_enter(*compRef, collision_info);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor_contact_b->name, e);
                }
            }
        }
    }
    else if(actor_contact_a->rb->has_collider && actor_contact_b->rb->has_collider){
        for (auto& [compName, compRef] : actor_contact_a->components) {
            if(!compRef->isTable()){
                continue;
            }
            bool enabled = (*compRef)["enabled"];
            if(!enabled) continue;
            auto on_collision_enter = (*compRef)["OnCollisionEnter"];
            if(on_collision_enter.isFunction()){
                try{
                    collision collision_info;
                    collision_info.other = actor_contact_b;
                    collision_info.point = contact_point;
                    collision_info.relative_velocity = rel_vel_a;
                    collision_info.normal = mani_normal;
                    on_collision_enter(*compRef, collision_info);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor_contact_a->name, e);
                }
            }
        }
        for (auto& [compName, compRef] : actor_contact_b->components) {
            if(!compRef->isTable()){
                continue;
            }
            bool enabled = (*compRef)["enabled"];
            if(!enabled) continue;
            auto on_collision_enter = (*compRef)["OnCollisionEnter"];
            if(on_collision_enter.isFunction()){
                try{
                    collision collision_info;
                    collision_info.other = actor_contact_a;
                    collision_info.point = contact_point;
                    collision_info.relative_velocity = rel_vel_a;
                    collision_info.normal = mani_normal;
                    on_collision_enter(*compRef, collision_info);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor_contact_b->name, e);
                }
            }
        }
    }
    
}
void ContactListener::EndContact(b2Contact *contact){
    Actor* actor_contact_a = reinterpret_cast<Actor*>(contact->GetFixtureA()->GetUserData().pointer);
    Actor* actor_contact_b = reinterpret_cast<Actor*>(contact->GetFixtureB()->GetUserData().pointer);
    b2Fixture *fixture_a = contact->GetFixtureA();
    b2Fixture *fixture_b = contact->GetFixtureB();
    b2WorldManifold world_manifold;
    contact->GetWorldManifold(&world_manifold);
    b2Vec2 contact_point = {-999.0f,-999.0f};
    b2Vec2 mani_normal = {-999.0f,-999.0f};
    b2Vec2 rel_vel_a = fixture_a->GetBody()->GetLinearVelocity() - fixture_b->GetBody()->GetLinearVelocity();
    if(actor_contact_a->rb->has_trigger && actor_contact_b->rb->has_trigger){
    //if(fixture_a->IsSensor() && fixture_b->IsSensor()){ // trigger
        for (auto& [compName, compRef] : actor_contact_a->components) {
            if(!compRef->isTable()){
                continue;
            }
            bool enabled = (*compRef)["enabled"];
            if(!enabled) continue;
            auto on_trigger_exit = (*compRef)["OnTriggerExit"];
            if(on_trigger_exit.isFunction()){
                try{
                    collision collision_info;
                    collision_info.other = actor_contact_b;
                    collision_info.point = {-999.0f,-999.0f};
                    collision_info.relative_velocity = rel_vel_a;
                    collision_info.normal = {-999.0f,-999.0f};
                    on_trigger_exit(*compRef, collision_info);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor_contact_a->name, e);
                }
            }
        }
        for (auto& [compName, compRef] : actor_contact_b->components) {
            if(!compRef->isTable()){
                continue;
            }
            bool enabled = (*compRef)["enabled"];
            if(!enabled) continue;
            auto on_trigger_exit = (*compRef)["OnTriggerExit"];
            if(on_trigger_exit.isFunction()){
                try{
                    collision collision_info;
                    collision_info.other = actor_contact_a;
                    collision_info.point = {-999.0f,-999.0f};
                    collision_info.relative_velocity = rel_vel_a;
                    collision_info.normal = {-999.0f,-999.0f};
                    on_trigger_exit(*compRef, collision_info);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor_contact_b->name, e);
                }
            }
        }
    }
    else if(actor_contact_a->rb->has_collider && actor_contact_b->rb->has_collider){
    //else if(!fixture_a->IsSensor() && !fixture_b->IsSensor()){ //collider
        for (auto& [compName, compRef] : actor_contact_a->components) {
            if(!compRef->isTable()){
                continue;
            }
            bool enabled = (*compRef)["enabled"];
            if(!enabled) continue;
            auto on_collision_exit = (*compRef)["OnCollisionExit"];
            if(on_collision_exit.isFunction()){
                try{
                    collision collision_info;
                    collision_info.other = actor_contact_b;
                    collision_info.point = contact_point;
                    collision_info.relative_velocity = rel_vel_a;
                    collision_info.normal = mani_normal;
                    on_collision_exit(*compRef, collision_info);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor_contact_a->name, e);
                }
            }
        }
        for (auto& [compName, compRef] : actor_contact_b->components) {
            if(!compRef->isTable()){
                continue;
            }
            bool enabled = (*compRef)["enabled"];
            if(!enabled) continue;
            auto on_collision_exit = (*compRef)["OnCollisionExit"];
            if(on_collision_exit.isFunction()){
                try{
                    collision collision_info;
                    collision_info.other = actor_contact_a;
                    collision_info.point = contact_point;
                    collision_info.relative_velocity = rel_vel_a;
                    collision_info.normal = mani_normal;
                    on_collision_exit(*compRef, collision_info);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor_contact_b->name, e);
                }
            }
        }
    }
}
