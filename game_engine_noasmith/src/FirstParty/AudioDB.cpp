//
//  AudioDB.cpp
//  game_engine
//
//  Created by Noah Smith on 2/8/26.
//

#include "AudioDB.hpp"


bool AudioDB::init() {
    if (AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        auto* platform = GetPlatformServices();
        std::string msg = std::string("Mix_OpenAudio failed: ") + ::Mix_GetError();
        if (platform) {
            platform->LogInfo(msg);
        } else {
            std::cout << msg << "\n";
        }
        return false;
    }
    AudioHelper::Mix_AllocateChannels(50);
    initialized = true;
    return true;
}
void AudioDB::AudioApi(lua_State *L){
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Audio")
        .addFunction("Play", PlayAudio)
        .addFunction("Halt", StopChannel)
        .addFunction("SetVolume", SetVolume)
        .endNamespace();
}
void AudioDB::LoadAudio(std::string &path) {
    auto* platform = GetPlatformServices();
    if (!platform) {
        PlatformInfoAndExit("error: platform services unavailable");
        return;
    }
    std::vector<std::string> files = platform->ListAssetFiles(path);
    for (const auto &file : files) {
        fs::path audio(file);
        std::string ext = audio.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (ext != ".wav" && ext != ".ogg") continue;
        std::string file_name = audio.string();
        Mix_Chunk* audio_file = AudioHelper::Mix_LoadWAV(file_name.c_str());
        if (!audio_file) {
            PlatformInfoAndExit("error: could not load audio file: " + file_name);
            return;
        }

        loaded_audio[audio.stem().string()] = audio_file;
    }
}
void AudioDB::PlayAudio( int channel,const std::string &soundRequested, bool loops) {
    int loop_int = 0;
    if(loops) loop_int = - 1;
    auto audio = loaded_audio.find(soundRequested);
    if(audio == loaded_audio.end()){
        PlatformInfoAndExit("error: failed to play audio clip " + soundRequested);
        return;
    }
    int audio_channel = AudioHelper::Mix_PlayChannel(channel, audio->second, loop_int);
    }
void AudioDB::SetVolume(int channel, int volume){
    AudioHelper::Mix_Volume(channel, volume);
}
void AudioDB::StopChannel(int channel) {
    AudioHelper::Mix_HaltChannel(channel);
}
void AudioDB::PauseChannel(int channel) {
    AudioHelper::Mix_Pause(channel);
}
void AudioDB::ResumeChannel(int channel) {
    AudioHelper::Mix_Resume(channel);
}
void AudioDB::Cleanup() {
    for (auto& pair : loaded_audio) {
        if (pair.second) {
            AudioHelper::Mix_FreeChunk(pair.second);
        }
    }
    loaded_audio.clear();
    AudioHelper::Mix_CloseAudio();
}
