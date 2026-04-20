#ifndef Template_hpp
#define Template_hpp
//
//  Template.hpp
//  game_engine
//
//  Created by Noah Smith on 2/2/26.
//
#include "Actor.hpp"
#include "EngineUtil.h"

class Template {
public:
    void LoadTemplateToActor(ComponentDB *component_db, std::string &template_name, Actor* actor_to_load);
private:
    bool FoundTemplate(std::string &template_name);
    bool TemplateLoaded(std::string& template_name);
    std::unordered_map<std::string, std::unique_ptr<Actor>> templates_loaded;
    
};
#endif
