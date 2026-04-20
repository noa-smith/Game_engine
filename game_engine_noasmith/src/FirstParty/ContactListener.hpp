//
//  ContactListener.hpp
//  game_engine
//
//  Created by Noah Smith on 3/31/26.
//
#include "EngineUtil.h"
#include "RigidBody2D.hpp"
class ContactListener : public b2ContactListener
{
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
};
