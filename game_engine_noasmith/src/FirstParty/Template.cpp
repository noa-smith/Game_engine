//
//  Template.cpp
//  game_engine
//
//  Created by Noah Smith on 2/2/26.
//


#include "Template.hpp"

void Template::LoadTemplateToActor(ComponentDB *component_db, std::string &template_name, Actor *actor_to_load) {
        rapidjson::Document doc;
        bool doc_loaded = LoadResources(doc, file_type::ActorTemplate, template_name);
        if (!doc_loaded) {
            PlatformInfoAndExit("error: Uncaught template load error: " + template_name);
            return;
        }
        std::string name = "";
        std::string components_dir = "resources/component_types/";
        if (doc.HasMember("name")) {
            actor_to_load->name = doc["name"].GetString();
        }
        //auto new_template = std::make_unique<Actor>(0, name);
        if (doc.HasMember("components") && doc["components"].IsObject()) {
            const auto &components_json = doc["components"];

            for (auto itr = components_json.MemberBegin(); itr != components_json.MemberEnd(); ++itr) {
                std::string component_id = itr->name.GetString();
                const auto &component_obj = itr->value;
                if (!component_obj.IsObject()) continue;
                if (!component_obj.HasMember("type") || !component_obj["type"].IsString()) continue;
                std::string component_type = component_obj["type"].GetString();
                component_db->LoadComponentType(component_type, components_dir);
                actor_to_load->AddLuaComponent(component_id, component_type, component_db);
                actor_to_load->AddKey(static_cast<const std::string>(component_id));
                for(auto comp_override = component_obj.MemberBegin(); comp_override != component_obj.MemberEnd(); ++comp_override){
                    if (std::strcmp(comp_override->name.GetString(), "type") == 0) continue;
                    if(comp_override->value.IsString()){
                        std::string message = comp_override->value.GetString();
                        actor_to_load->Override(component_id, comp_override->name.GetString(), message);
                    }
                    else if(comp_override->value.IsFloat()){
                        actor_to_load->Override(component_id, comp_override->name.GetString(), comp_override->value.GetFloat());
                    }
                    else if(comp_override->value.IsInt()){
                        actor_to_load->Override(component_id, comp_override->name.GetString(), comp_override->value.GetInt());
                    }
                    else if(comp_override->value.IsBool()){
                        actor_to_load->Override(component_id, comp_override->name.GetString(), comp_override->value.GetBool());
                    }
                    else{
                        PlatformInfoAndExit("Unsupported Type at:" + component_id + ", " + comp_override->name.GetString());
                        return;
                    }
                    
                //}
            }
        }
        //templates_loaded.emplace(template_name, std::move(new_template));
    }
    //return templates_loaded.find(template_name)->second.get();
};
