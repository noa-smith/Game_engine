#ifndef RIGIDBODY2D_HPP
#define RIGIDBODY2D_HPP
#include "EngineUtil.h"

struct Actor;
struct collision {
    Actor *other;
    b2Vec2 point;
    b2Vec2 relative_velocity;
    b2Vec2 normal;
    
};
class RigidBody2D {
public:
    Actor* actor_ref = nullptr;
    std::string type = "Rigidbody";
    std::string key = "";
    bool enabled = true;
    bool removed = false;
    bool OnStartRan = false;
    
    float x = 0.0f;
    float y = 0.0f;
    std::string body_type = "dynamic";
    bool precise = true;
    float gravity_scale = 1.0f;
    float density = 1.0f;
    float angular_friction = 0.3f;
    float rotation = 0.0f;
    bool has_collider = true;
    bool has_trigger = true;
    std::string collider_type = "box";
    b2Body* m_body = nullptr;
    float width = 1.0f;
    float height = 1.0f;
    float radius = 0.5f;
    float friction = 0.3f;
    float bounciness = 0.3f;
    
    std::string trigger_type = "box";
    float trigger_width = 1.0f;
    float trigger_height = 1.0f;
    float trigger_radius = 0.5f;
public:
    RigidBody2D() = default;
    ~RigidBody2D() = default;
    void SetActor(Actor *a);
    void OnStart(b2World* world);
    void Destroy(b2World* world);

    b2Vec2 GetPosition() const;
    float GetRotation() const;
    void AddForce(const b2Vec2& vector);
    void SetVelocity(const b2Vec2& vector);
    void SetPosition(const b2Vec2& vector);
    void SetRotation(const float& deg);
    void SetAngularVelocity(const float& deg);
    void SetGravityScale(const float& grav);
    void SetUpDirection(const b2Vec2& vector);
    void SetRightDirection(const b2Vec2& vector);
    void SetLinearVelocity(const b2Vec2& v);
    b2Vec2 GetVelocity() const;
    float GetAngularVelocity() const;
    float GetGravityScale() const;
    b2Vec2 GetUpDirection() const;
    b2Vec2 GetRightDirection() const;

    b2Body* GetBody() const { return m_body; }

//private:
    
};

#endif
