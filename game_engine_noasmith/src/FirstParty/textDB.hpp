//
//  textDB.hpp
//  game_engine
//
//  Created by Noah Smith on 2/7/26.
//
#ifndef TEXTDB_HPP
#define TEXTDB_HPP
#include "EngineUtil.h"


class textDB {
public:
    textDB(SDL_Renderer *gr, TTF_Font *default_font)
        : game_renderer(gr)
        , default_font(default_font) {}
    void LoadIntroText();
    static void LoadTextTexture(SDL_Renderer *renderer, TTF_Font *font, const std::string &content, SDL_Color color);
    static SDL_Texture *CreateTextTexture(SDL_Renderer *renderer, TTF_Font *font, const std::string &content,
                                          SDL_Color color, float &outW, float &outH);
    SDL_Texture *GetTexture(int key_in);
    SDL_Color default_font_color = { 255, 255, 255, 255 };
    int default_font_size = 16;
    TTF_Font *default_font;
    void ClearTextQueue();
    void ClearTextTextureCache();
    void TextApi(lua_State *L);
    std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> font_data_base;
private:
    std::unordered_map<int, SDL_Texture *> intro_text;
    
    SDL_Renderer *game_renderer;
};

namespace TextRenderer {
void DrawText(const std::string &text_content,float x, float y, const std::string &font_name, int font_size, int r, int g, int b, int a);
void RenderTextQueue(SDL_Renderer *game_renderer, textDB *text_data_base);
}
#endif
