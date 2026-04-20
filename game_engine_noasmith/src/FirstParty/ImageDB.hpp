//
//  ImageDB.hpp
//  game_engine
//
//  Created by Noah Smith on 2/5/26.
//
#ifndef Imagedb_hpp
#define Imagedb_hpp
#include "EngineUtil.h"

struct TextureInfo {
    SDL_Texture *texture;
    float width;
    float height;
};
class ImageDB {
public:
    ImageDB(SDL_Renderer *gr)
        : game_renderer(gr) {}
    void LoadImages();
    TextureInfo GetTexture(std::string &key_in);
    

private:
    std::unordered_map<std::string, TextureInfo> loaded_images;
    SDL_Renderer *game_renderer;
};

#endif
