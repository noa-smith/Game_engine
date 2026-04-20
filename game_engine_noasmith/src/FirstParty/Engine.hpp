//
//  Engine.hpp
//  game_engine
//
//  Created by Noah Smith on 1/21/26.
//

#ifndef Engine_hpp
#define Engine_hpp

#include <string>

#include <stdio.h>
#include "PlatformServices.hpp"
#include "PlatformContext.hpp"
#include "Input.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "ImageDB.hpp"
#include "textDB.hpp"
#include "AudioDB.hpp"
#include "Physics.hpp"


class Engine {
public:
    Engine();
    void GameLoop();
    Scene gameScene;
private:
    void OnStart();
    void OnUpdate();
    void OnLateUpdate();
    void OnDestroy();
    GameState game_state;
    Renderer GameWindow;
    std::unique_ptr<ImageDB> ImageDataBase;
    std::unique_ptr<textDB> TextDataBase;
    std::unique_ptr<AudioDB> AudioDataBase;
    std::unique_ptr<PlatformServices> platform;
    void BeginGame();
    void SetCamYOffset();
    void SetCamXOffset();
    void MoveActorPos();
    void CheckCoolDown();
    void CheckActorDir();
    void Vector2();
    int CAMERA_HEIGHT = 0;
    int CAMERA_WIDTH = 0;
    int CAMERA_HEIGHT_HALF = 0;
    int CAMERA_WIDTH_HALF = 0;
    bool running = false;
};


// Provide a user-defined default constructor now that all member types are complete.
inline Engine::Engine()
    : gameScene()
    , game_state()
    , GameWindow()
    , ImageDataBase(nullptr)
    , TextDataBase(nullptr)
    , AudioDataBase(nullptr)
    , CAMERA_HEIGHT(0)
    , CAMERA_WIDTH(0)
    , CAMERA_HEIGHT_HALF(0)
    , CAMERA_WIDTH_HALF(0)
    , running(false) {}

#endif /* Engine_hpp */
