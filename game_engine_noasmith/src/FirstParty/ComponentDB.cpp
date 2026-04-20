#include "ComponentDB.hpp"


void ComponentDB::SetLuaState(lua_State *state) {
    this->lua_state = state;
};

void ComponentDB::Clear() {
    base_types.clear();
    source_files.clear();
};

bool ComponentDB::HasType(const std::string &type_name) {
    return base_types.find(type_name) != base_types.end();
};

luabridge::LuaRef ComponentDB::GetBaseType(const std::string &type_name) {
    auto it = base_types.find(type_name);
    if (it == base_types.end() || !it->second) {
        return luabridge::LuaRef(lua_state);
    }
    return *(it->second);
};

bool ComponentDB::LoadComponentType(const std::string &type_name, const std::string &file_path) {
    if (lua_state == nullptr) {
        auto *platform = GetPlatformServices();
        std::string msg = "error: ComponentDB lua_State is null";
        if (platform) {
            platform->LogInfo(msg);
        } else {
            std::cout << msg << "\n";
        }
        return false;
    }
    if (type_name == "Rigidbody") {
        return true;
    }
    auto existing = source_files.find(type_name);
    if (existing != source_files.end() && existing->second == file_path) {
        return true;
    }
    return LoadFileAndCache(type_name, file_path);
}

bool ComponentDB::LoadFileAndCache(const std::string &type_name, const std::string &file_path) {
    std::string lua_file = file_path + type_name + ".lua";
    std::string lua_src = ReadTextAsset(lua_file);
    if (lua_src.empty()) {
        PlatformInfoAndExit("error: failed to locate component " + type_name);
        return false;
    }
    if (luaL_loadbuffer(lua_state, lua_src.c_str(), lua_src.size(), lua_file.c_str()) != LUA_OK
        || lua_pcall(lua_state, 0, 0, 0) != LUA_OK) {
        PlatformInfoAndExit("problem with lua file " + type_name);
        lua_pop(lua_state, 1);
        return false;
    }

    luabridge::LuaRef base = luabridge::getGlobal(lua_state, type_name.c_str());
    if (!base.isTable()) {
        auto *platform = GetPlatformServices();
        std::string msg = "error: lua base table '" + type_name + "' missing";
        if (platform) {
            platform->LogInfo(msg);
        } else {
            std::cout << msg << "\n";
        }
        return false;
    }

    base_types[type_name] = std::make_shared<luabridge::LuaRef>(base);
    source_files[type_name] = file_path;
    return true;
}

luabridge::LuaRef ComponentDB::CreateInstance(const std::string &type_name) {
    if (lua_state == nullptr) {
        return luabridge::LuaRef(lua_state);
    }
    if (type_name == "Rigidbody") {
        auto *body = new RigidBody2D();
        luabridge::LuaRef instance(lua_state, body);
        return instance;
    }
    auto it = base_types.find(type_name);
    if (it == base_types.end() || !it->second) {
        return luabridge::LuaRef(lua_state);
    }

    luabridge::LuaRef instance = luabridge::newTable(lua_state);
    EstablishInheritance(instance, *(it->second), type_name);
    return instance;
}

bool ComponentDB::EstablishInheritance(luabridge::LuaRef &instance, luabridge::LuaRef &base, const std::string &type) {
    if (!base.isTable() || !instance.isTable()) {
        return false;
    }
    luabridge::LuaRef meta = luabridge::newTable(instance.state());
    meta["__index"] = base;
    instance["type"] = type;
    instance.push(lua_state);
    meta.push(lua_state);
    lua_setmetatable(lua_state, -2);
    lua_pop(lua_state, 1);
    return true;
}
