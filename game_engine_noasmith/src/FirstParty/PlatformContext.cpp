//
//  PlatformContext.cpp
//  game_engine
//
//  Created by Noah Smith on 4/12/26.
//

#include "PlatformContext.hpp"
static PlatformServices* gPlatform = nullptr;

void SetPlatformServices(PlatformServices* p) { gPlatform = p; }
PlatformServices* GetPlatformServices() { return gPlatform; }
