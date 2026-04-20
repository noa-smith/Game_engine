//
//  ImageDB.cpp
//  game_engine
//
//  Created by Noah Smith on 2/5/26.
//


#include "ImageDB.hpp"

struct TextureInfo;



void ImageDB::LoadImages() {
    auto* platform = GetPlatformServices();
    if (!platform) {
        PlatformInfoAndExit("error: platform services unavailable");
        return;
    }
    std::string resource_path = "resources/images";
    for (const auto &img_file : platform->ListAssetFiles(resource_path)) {
        fs::path image_path(img_file);
        std::string ext = image_path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (ext != ".png") {
            continue;
        }

        SDL_RWops* rw = SDL_RWFromFile(image_path.string().c_str(), "rb");
        if (!rw) {
            continue;
        }
        SDL_Texture *img_to_load = IMG_LoadTexture_RW(game_renderer, rw, 1);
        if (img_to_load) {
            float w = 0.0f, h = 0.0f;
            Helper::SDL_QueryTexture(img_to_load, &w, &h);

            std::string img_key = image_path.stem().string();
            loaded_images[img_key] = TextureInfo{ img_to_load, w, h };
        }
    }
}
TextureInfo ImageDB::GetTexture(std::string &key_in) {
    auto img_req = loaded_images.find(key_in);
    if (img_req == loaded_images.end()) {
        PlatformInfoAndExit("error: missing image " + key_in);
        return TextureInfo{nullptr, 0.0f, 0.0f};
    }
    return img_req->second;
}
