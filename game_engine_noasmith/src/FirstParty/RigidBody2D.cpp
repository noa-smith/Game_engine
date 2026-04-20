//
//  RigidBody2D.cpp
//  game_engine
//
//  Created by Noah Smith on 3/27/26.
//
#include "RigidBody2D.hpp"
#include "Actor.hpp"

const float radians = (b2_pi / 180.0f);
const float degrees = (180.0f / b2_pi);

void RigidBody2D::SetActor(Actor* a) { actor_ref = a; }
void RigidBody2D::OnStart(b2World* world) {
    if (!world || m_body) return;

    b2BodyDef def;
    if(body_type == "dynamic"){
        def.type = b2_dynamicBody;
    } else if (body_type == "static"){
        def.type = b2_staticBody;
    }
    else if (body_type == "kinematic"){
        def.type = b2_kinematicBody;
    }
    def.position.Set(x, y);
    def.bullet = precise;
    def.gravityScale = gravity_scale;
    def.angularDamping = angular_friction;

    def.angle = rotation * radians;

    m_body = world->CreateBody(&def);
    b2PolygonShape box_shape;
    b2CircleShape circle_shape;
    b2FixtureDef fixture_def;
    if(!has_trigger && !has_collider){
        fixture_def.isSensor = true;
        if(collider_type == "box"){
            box_shape.SetAsBox(0.5f * width, 0.5f * height);
            fixture_def.shape = &box_shape;
        }
        else if(collider_type == "circle"){
            circle_shape.m_radius = radius;
            fixture_def.shape = &circle_shape;
        }
        fixture_def.density = density;
        fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(this->actor_ref);
        m_body->CreateFixture(&fixture_def);
        return;
    }
    b2FixtureDef trigger_fixture_def;
    b2PolygonShape trigger_box_shape;
    b2CircleShape trigger_circle_shape;
    if(has_collider){
        if(collider_type == "box"){
            box_shape.SetAsBox(0.5f * width, 0.5f * height);
            fixture_def.shape = &box_shape;
        }
        else if(collider_type == "circle"){
            circle_shape.m_radius = radius;
            fixture_def.shape = &circle_shape;
        }
        else {
            PlatformInfoAndExit("invalid shape");
            return;
        }
        fixture_def.density = density;
        fixture_def.friction = friction;
        fixture_def.restitution = bounciness;
        fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(this->actor_ref);
        m_body->CreateFixture(&fixture_def);
    }
    if(has_trigger){
        trigger_fixture_def.isSensor = true;
        if(trigger_type == "box"){
            trigger_box_shape.SetAsBox(0.5f * trigger_width, 0.5f * trigger_height);
            trigger_fixture_def.shape = &trigger_box_shape;
        }
        else if(trigger_type == "circle"){
            trigger_circle_shape.m_radius = trigger_radius;
            trigger_fixture_def.shape = &trigger_circle_shape;
        }
        else {
            PlatformInfoAndExit("invalid shape");
            return;
        }
        trigger_fixture_def.density = density;
        trigger_fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(this->actor_ref);
        m_body->CreateFixture(&trigger_fixture_def);
    }
}
b2Vec2 RigidBody2D::GetPosition() const{
    return m_body->GetPosition();
}
float RigidBody2D::GetRotation() const {
    return m_body->GetAngle() * (degrees);
}
void RigidBody2D::AddForce(const b2Vec2& vector){
    m_body->ApplyForceToCenter(vector, true);
}
void RigidBody2D::SetVelocity(const b2Vec2& vector){
    m_body->SetLinearVelocity(vector);
}
void RigidBody2D::SetPosition(const b2Vec2& vector){
    m_body->SetTransform(vector, m_body->GetAngle());
}
void RigidBody2D::SetRotation(const float& deg){
    const float deg_to_rad = deg * (radians);
    m_body->SetTransform(m_body->GetPosition(), deg_to_rad);
}
void RigidBody2D::SetAngularVelocity(const float& deg){
    float deg_to_rad = deg * (radians);
    m_body->SetAngularVelocity(deg_to_rad);
}
void RigidBody2D::SetGravityScale(const float& grav){
    m_body->SetGravityScale(grav);
}
b2Vec2 RigidBody2D::GetVelocity() const {
    return m_body->GetLinearVelocity();
}
float RigidBody2D::GetAngularVelocity() const{
    return m_body->GetAngularVelocity() * (180.0f / b2_pi);
}
float RigidBody2D::GetGravityScale() const{
    return m_body->GetGravityScale();
}
b2Vec2 RigidBody2D::GetUpDirection() const{
    float angle = m_body->GetAngle();
    b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle)); //maybe
    result.Normalize();
    return result;
}

void RigidBody2D::SetRightDirection(const b2Vec2& dir) {
    b2Vec2 d = dir;
    if (d.LengthSquared() == 0.0f) return;
    d.Normalize();

    const float angle = glm::atan(d.y, d.x);
    m_body->SetTransform(m_body->GetPosition(), angle);
}

b2Vec2 RigidBody2D::GetRightDirection() const {
    const float a = m_body->GetAngle();
    return b2Vec2(glm::cos(a), glm::sin(a));
}

void RigidBody2D::SetUpDirection(const b2Vec2& up) {
    b2Vec2 u = up;
    if (u.LengthSquared() == 0.0f) return;
    u.Normalize();
    const float angle = glm::atan(u.y, u.x) - b2_pi * 0.5f;
    m_body->SetTransform(m_body->GetPosition(), angle);
}
