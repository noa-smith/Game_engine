//
//  Engine.cpp
//  game_engine
//
//  Created by Noah Smith on 1/21/26.
//

#include "Engine.hpp"
#include "DesktopPlatformServices.hpp"
#include "AndroidPlatformServices.hpp"

// Wrapper to expose Physics::RaycastAll as a Lua-friendly function returning a table
static luabridge::LuaRef RaycastAllLua(b2Vec2 pos, b2Vec2 dir, float dist) {
    lua_State* L = Scene::current->GetLuaState();
    auto hits = Physics::RaycastAll(pos, dir, dist);
    luabridge::LuaRef arr = luabridge::newTable(L);
    int index = 1; // 1-indexed as per Lua convention/spec
    for (const auto& h : hits) {
        arr[index++] = h; // HitResult is registered as a class
    }
    return arr;
}

int main(int argc, char *argv[]) {
    Engine engine;
    engine.GameLoop();  // run Game Engine with loop
    return 0;
}


void Engine::GameLoop() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return;
    }
    #if defined(__ANDROID__)
    platform = std::make_unique<AndroidPlatformServices>();
    #else
    platform = std::make_unique<DesktopPlatformServices>();
    #endif
    SetPlatformServices(platform.get());
    gameScene.StartLua();
    Vector2();
    Input::Init(gameScene.component_data_base.lua_state);
    BeginGame();
    while (running) {
        if(gameScene.load_new_scene){
            gameScene.LoadNewScene();
        }
        OnStart();
        GameWindow.GamePoll(running, game_state);
        if (!GameWindow.IsSuspended()) {
            GameWindow.ClearWindow();
        }
        OnUpdate();
        OnLateUpdate();
        OnDestroy();
        int frame_num = Helper::GetFrameNumber();
        Input::LateUpdate();
        gameScene.StartInstantiated();
        gameScene.DestroyActorQueue();
        if(gameScene.current->world_created){
            gameScene.world->Step(1.0f / 60.0f, 8, 3);
        }
        ActorRenderer::RenderActorQueue(GameWindow, ImageDataBase.get(), ZERO);
        TextRenderer::RenderTextQueue(GameWindow.RenderPtr(), TextDataBase.get());
        ActorRenderer::RenderPixelQueue(GameWindow, ImageDataBase.get(), ZERO);
        if (!GameWindow.IsSuspended()) {
            GameWindow.RenderFrame();
        }
        if (game_state == GameState::Quit) {
            break;
        }
    }
    AudioDB::Cleanup();
    SDL_Quit();
}

void Engine::OnStart() {
    for (auto &actor : gameScene.actors) {
        if(!actor) continue;
        for (auto &[name, comp] : actor->components) {
            if (!comp->isTable()) {
                continue;
            }
            auto onStart = (*comp)["OnStart"];
            bool ran = (*comp)["OnStartRan"];
            bool enabled = (*comp)["enabled"];
            if(!ran && enabled){
                (*comp)["OnStartRan"] = true;
                if (onStart.isFunction()) {
                    try {
                        onStart(*comp);
                        
                    }
                    catch(luabridge::LuaException &e){
                        ReportError(actor->name, e);
                    }
                }
            }
        }
    }
}
void Engine::OnUpdate(){
    for (auto &actor : gameScene.actors){
        if(!actor) continue;
        for(auto &[name, comp] : actor->components){
            if(!comp->isTable()){
                continue;
            }
            bool enabled = (*comp)["enabled"];
            if(!enabled) continue;
            auto on_update = (*comp)["OnUpdate"];
            if(on_update.isFunction()){
                try{
                    on_update(*comp);
                }
                catch(const luabridge::LuaException &e) {
                    ReportError(actor->name, e);
                }
            }
        }
    }
}
void Engine::OnLateUpdate(){
    for (auto &actor : gameScene.actors){
        if(!actor) continue;
        for(auto &[name, comp] : actor->components){
            if(!comp->isTable()){
                continue;
            }
            bool enabled = (*comp)["enabled"];
            if(!enabled) continue;
            auto on_update_late = (*comp)["OnLateUpdate"];
            if(on_update_late.isFunction()){
                try{
                    on_update_late(*comp);
                }
                catch(luabridge::LuaException &e){
                    ReportError(actor->name, e);
                }
                
            }
        }
        
    }
}
void Engine::OnDestroy(){
    b2World* world = gameScene.current ? gameScene.current->world : nullptr;
    for(auto &actor : gameScene.actors){
        if(!actor) continue;
        for(auto& [name, comp] : actor->components){
            if(!comp->isTable() && !comp->isUserdata()){
                continue;
            }
            
            bool destroy = (*comp)["removed"];
            if(!destroy && !actor->destroyed) continue;

            if (world != nullptr && comp->isUserdata()) {
                try {
                    RigidBody2D* rb = comp->cast<RigidBody2D*>();
                    if (rb != nullptr && rb->m_body != nullptr) {
                        world->DestroyBody(rb->m_body);
                        rb->m_body = nullptr;
                    }
                } catch (const luabridge::LuaException&) {
                }
            }

            auto on_destroy = (*comp)["OnDestroy"];
            if(on_destroy.isFunction()){
                try {
                    on_destroy(*comp);
                }
                catch (luabridge::LuaException(&e)){
                    ReportError(actor->name, e);
                }
            }
        }
        actor->MoveAddedComponents();
        
        actor->ExecuteRemovals();
        
    }
}
void Engine::BeginGame() {
    gameScene.LoadScene(GameWindow);
    SetCamXOffset();
    SetCamYOffset();
    ImageDataBase = std::make_unique<ImageDB>(GameWindow.RenderPtr());
    TextDataBase = std::make_unique<textDB>(GameWindow.RenderPtr(), gameScene.game_font);
    AudioDataBase = std::make_unique<AudioDB>();
    TextDataBase->default_font = gameScene.game_font;
    TextDataBase->TextApi(gameScene.component_data_base.lua_state);
    ImageDataBase->LoadImages();  // load all images
    if (AudioDataBase->init()) {
        std::string audio_resources = "resources/audio/";
        AudioDataBase->LoadAudio(audio_resources);
        AudioDataBase->AudioApi(gameScene.component_data_base.lua_state);
        GameWindow.ImageApi(gameScene.component_data_base.lua_state);
        GameWindow.CameraApi(gameScene.component_data_base.lua_state);
    }
    ContactListener* contact_listener = new ContactListener();
    GameWindow.SetImageDB(ImageDataBase.get());
    if(gameScene.current->world == nullptr){
        gameScene.current->world = new b2World(b2Vec2(0.0f, 0.0f));
    }
    if (gameScene.current->world != nullptr) {
        gameScene.current->world->SetContactListener(contact_listener);
        for (auto& actor : gameScene.actors) {
            if (!actor) continue;
            for (auto& [compName, compRef] : actor->components) {
                if (!compRef) continue;
                luabridge::LuaRef& comp = *compRef;
                // Prefer checking declared type when available
                luabridge::LuaRef typeRef = comp["type"];
                bool isRigidbody = false;
                if (typeRef.isString()) {
                    isRigidbody = (typeRef.cast<std::string>() == "Rigidbody");
                }
                if (!isRigidbody && comp.isUserdata()) {
                    try {
                        RigidBody2D* rbPtr = comp.cast<RigidBody2D*>();
                        if (rbPtr != nullptr) {
                            isRigidbody = true;
                        }
                    } catch (...) {
                        // Not a RigidBody2D userdata; skip
                    }
                }
                if (!isRigidbody) continue;
                try {
                    RigidBody2D* rb = comp.cast<RigidBody2D*>();
                    if (rb) {
                        rb->OnStart(gameScene.current->world);
                    }
                } catch (const luabridge::LuaException&) {
                }
            }
        }
    }
    running = true;
}


void Engine::SetCamYOffset() {
    CAMERA_HEIGHT = gameScene.CAMERA_HEIGHT;
    CAMERA_HEIGHT_HALF = (CAMERA_HEIGHT - 1) / 2;
}


void Engine::SetCamXOffset() {
    CAMERA_WIDTH = gameScene.CAMERA_WIDTH;
    CAMERA_WIDTH_HALF = (CAMERA_WIDTH - 1) / 2;
}

void Engine::Vector2(){
    luabridge::getGlobalNamespace(gameScene.GetLuaState())
        .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*) (float, float)>()
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addFunction("__add", &b2Vec2::operator_add)
        .addFunction("__sub", &b2Vec2::operator_sub)
        .addFunction("__mul", &b2Vec2::operator_mul)
        .endClass();
    luabridge::getGlobalNamespace(gameScene.GetLuaState())
        .beginNamespace("Vector2")
        .addFunction("Distance", &b2Distance)
        .addFunction(
                    "Dot",
                    static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot)
                )
        .endNamespace();
    luabridge::getGlobalNamespace(gameScene.GetLuaState())
        .beginNamespace("Physics")
        .addFunction("Raycast", &Physics::Raycast)
        .addFunction("RaycastAll", &RaycastAllLua)
        .endNamespace();
    luabridge::getGlobalNamespace(gameScene.GetLuaState())
        .beginClass<HitResult>("HitResult")
        .addProperty("actor", &HitResult::actor)
        .addProperty("point", &HitResult::point)
        .addProperty("normal", &HitResult::normal)
        .addProperty("is_trigger", &HitResult::is_trigger)
        .endClass();
    luabridge::getGlobalNamespace(gameScene.GetLuaState())
        .beginClass<RigidBody2D>("Rigidbody")
        .addConstructor<void(*) (void)>()
        .addData("actor", &RigidBody2D::actor_ref)
        .addData("type", &RigidBody2D::type)
        .addData("key", &RigidBody2D::key)
        .addData("enabled", &RigidBody2D::enabled)
        .addData("removed", &RigidBody2D::removed)
        .addData("OnStartRan", &RigidBody2D::OnStartRan)
        // Setup-time physics fields
        .addData("x", &RigidBody2D::x)
        .addData("y", &RigidBody2D::y)
        .addData("body_type", &RigidBody2D::body_type)
        .addData("precise", &RigidBody2D::precise)
        .addData("gravity_scale", &RigidBody2D::gravity_scale)
        .addData("density", &RigidBody2D::density)
        .addData("angular_friction", &RigidBody2D::angular_friction)
        .addData("rotation", &RigidBody2D::rotation)
        .addData("has_collider", &RigidBody2D::has_collider)
        .addData("has_trigger", &RigidBody2D::has_trigger)
        .addData("width", &RigidBody2D::width)
        .addData("height", &RigidBody2D::height)
        .addData("radius", &RigidBody2D::radius)
        .addData("friction", &RigidBody2D::friction)
        .addData("bounciness", &RigidBody2D::bounciness)
        .addData("collider_type", &RigidBody2D::collider_type)
        .addData("trigger_type", &RigidBody2D::trigger_type)
        .addData("trigger_width", &RigidBody2D::trigger_width)
        .addData("trigger_height", &RigidBody2D::trigger_height)
        .addData("trigger_radius", &RigidBody2D::trigger_radius)
        .addFunction("GetPosition", &RigidBody2D::GetPosition)
        .addFunction("GetRotation", &RigidBody2D::GetRotation)
        .addFunction("AddForce", &RigidBody2D::AddForce)
        .addFunction("SetVelocity", &RigidBody2D::SetVelocity)
        .addFunction("SetPosition", &RigidBody2D::SetPosition)
        .addFunction("SetRotation", &RigidBody2D::SetRotation)
        .addFunction("SetAngularVelocity", &RigidBody2D::SetAngularVelocity)
        .addFunction("SetGravityScale", &RigidBody2D::SetGravityScale)
        .addFunction("SetUpDirection", &RigidBody2D::SetUpDirection)
        .addFunction("SetRightDirection", &RigidBody2D::SetRightDirection)
        .addFunction("AddForce", &RigidBody2D::AddForce)
        .addFunction("SetVelocity", &RigidBody2D::SetVelocity)
        .addFunction("GetVelocity", &RigidBody2D::GetVelocity)
        .addFunction("GetAngularVelocity", &RigidBody2D::GetAngularVelocity)
        .addFunction("GetGravityScale", &RigidBody2D::GetGravityScale)
        .addFunction("GetUpDirection", &RigidBody2D::GetUpDirection)
        .addFunction("GetRightDirection", &RigidBody2D::GetRightDirection)
        .endClass();
    luabridge::getGlobalNamespace(gameScene.GetLuaState())
        .beginClass<collision>("collision")
            
            .addProperty("other", &collision::other)
            .addProperty("point", &collision::point)
            .addProperty("relative_velocity", &collision::relative_velocity)
            .addProperty("normal", &collision::normal)
            .endClass();
}
