//
//  ComponentDB.hpp
//  game_engine
//
//  Created by Noah Smith on 2/26/26.
//#include "ComponentDB.hpp"
#ifndef COMPONENTDB_HPP
#define COMPONENTDB_HPP
#include <iostream>
#include <map>
#include <string>
#include "RigidBody2D.hpp"

class ComponentDB {
public:
    ComponentDB() = default;
    ComponentDB(lua_State *lua_state) : lua_state(lua_state) {};
    void SetLuaState(lua_State *lua_state);
    void Clear();
    bool HasType(const std::string &type_name);
    luabridge::LuaRef GetBaseType(const std::string &type_name);
    bool LoadComponentType(const std::string &type_name, const std::string &file_path);
    luabridge::LuaRef CreateInstance(const std::string &type_name);
    lua_State *lua_state;
private:
    bool LoadFileAndCache(const std::string &type_name, const std::string &file_path);
    bool EstablishInheritance(luabridge::LuaRef &instance, luabridge::LuaRef &base, const std::string &type);
    
    std::map<std::string, std::shared_ptr<luabridge::LuaRef>> base_types;
    std::map<std::string, std::string> source_files;
};



#endif

