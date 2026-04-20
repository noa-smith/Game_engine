//
//  AudioDB.hpp
//  game_engine
//
//  Created by Noah Smith on 2/8/26.
//

#ifndef AUDIODB_HPP
#define AUDIODB_HPP
#include "EngineUtil.h"

class AudioDB {
public:
    bool init();
    void LoadAudio(std::string &path);
    void AudioApi(lua_State *L);
    static void PlayAudio( int channel,const std::string &soundRequested, bool loops);
    static void SetVolume(int channel, int volume);
    static void StopChannel(int channel);
    static void PauseChannel(int channel);
    static void ResumeChannel(int channel);
    static void Cleanup();
private:
    bool initialized = false;
    static inline std::unordered_map<std::string, Mix_Chunk*> loaded_audio;
};


#endif
