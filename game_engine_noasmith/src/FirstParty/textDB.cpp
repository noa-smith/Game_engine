//
//  textDB.cpp
//  game_engine
//
//  Created by Noah Smith on 2/7/26.
//

#include "textDB.hpp"

#include <string>
#include <unordered_map>

struct DrawTextRequest {
    const std::string content;
    const std::string font_name;
    int font_size;
    SDL_Color color;
    glm::vec2 coord;
};

struct TextTextureInfo {
    SDL_Texture *texture;
    float width;
    float height;
};

static std::vector<DrawTextRequest> draw_text_queue;
static std::unordered_map<std::string, TextTextureInfo> text_texture_cache;

void TextRenderer::DrawText(const std::string &text_content,float x, float y, const std::string &font_name, int font_size, int r, int g, int b, int a) {
    SDL_Color font_color = {static_cast<Uint8>(r),static_cast<Uint8>(g),static_cast<Uint8>(b),static_cast<Uint8>(a)};
    draw_text_queue.push_back(DrawTextRequest { text_content, font_name, font_size, font_color, glm::vec2(x, y) });
}

void textDB::ClearTextQueue() {
    draw_text_queue.clear();
}

void textDB::TextApi(lua_State *L){
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Text")
        .addFunction("Draw", TextRenderer::DrawText)
        .endNamespace();
}

void textDB::ClearTextTextureCache() {
    for (auto &entry : text_texture_cache) {
        if (entry.second.texture) {
            SDL_DestroyTexture(entry.second.texture);
        }
    }
    text_texture_cache.clear();
}


SDL_Texture *textDB::CreateTextTexture(SDL_Renderer *renderer, TTF_Font *font, const std::string &content,
                                       SDL_Color color, float &outW, float &outH) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, content.c_str(), color);
    if (!surface) {
        return nullptr;
    }
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    outW = static_cast<float>(surface->w);
    outH = static_cast<float>(surface->h);
    SDL_FreeSurface(surface);
    return text_texture;
}


void textDB::LoadTextTexture(SDL_Renderer *renderer, TTF_Font *font, const std::string &content, SDL_Color color) {
    float outW;
    float outH;
    SDL_Texture *text_texture = textDB::CreateTextTexture(renderer, font, content, color, outW, outH);
    if (text_texture) {
        text_texture_cache.emplace(content, TextTextureInfo { text_texture, outW, outH });
    }
}


void TextRenderer::RenderTextQueue(SDL_Renderer *game_renderer, textDB *text_data_base) {
    for (const auto &req : draw_text_queue) {
        std::string key = req.content;

        SDL_Texture *text_texture = nullptr;
        float w = 0.0f, h = 0.0f;

        auto it = text_texture_cache.find(key);
        if (it != text_texture_cache.end()) {
            text_texture = it->second.texture;
            w = it->second.width;
            h = it->second.height;
        } else {
            TTF_Font *text_font = nullptr;
            auto font = text_data_base->font_data_base.find(req.font_name);
            if(font != text_data_base->font_data_base.end()){
                auto font_size = font->second.find(req.font_size);
                if(font_size != font->second.end()){
                    text_font = font_size->second;
                }
            }
            if(!text_font){
                fs::path font_path = fs::path("resources/fonts") / (req.font_name + ".ttf");
                SDL_RWops* rw = SDL_RWFromFile(font_path.string().c_str(), "rb");
                text_font = rw ? TTF_OpenFontRW(rw, 1, req.font_size) : nullptr;
                if (!text_font) {
                    std::string msg = "error: font " + font_path.filename().stem().string() + " missing";
                    PlatformInfoAndExit(msg);
                    return;
                }
                text_data_base->font_data_base[req.font_name][req.font_size] = text_font;
            }
            text_texture
              = textDB::CreateTextTexture(game_renderer, text_font, req.content, req.color, w, h);
            if (text_texture) {
                text_texture_cache.emplace(key, TextTextureInfo { text_texture, w, h });
            } else {
                continue;
            }
        }

        SDL_FRect rect;
        rect.x = static_cast<float>(req.coord.x);
        rect.y = static_cast<float>(req.coord.y);
        rect.w = w;
        rect.h = h;
        Helper::SDL_RenderCopy(game_renderer, text_texture, NULL, &rect);
    }
    draw_text_queue.clear();
}
