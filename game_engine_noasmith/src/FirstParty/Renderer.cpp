//
//  Renderer.cpp
//  game_engine
//
//  Created by Noah Smith on 2/4/26.
//
#include "Renderer.hpp"

Renderer::~Renderer() {
    if (render != nullptr) {
        SDL_DestroyRenderer(render);
        render = nullptr;
    }
    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void Renderer::StartRenderer() {
    StartWindow();
    ClearWindow();
}
void Renderer::SetImageDB(ImageDB * img_db){
    image_data_base_ptr = img_db;
}
SDL_Renderer *Renderer::RenderPtr() {
    return this->render;
}

void Renderer::ImageApi(lua_State *L){
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Image")
        .addFunction("Draw", &Renderer::DrawApi)
        .addFunction("DrawEx", &Renderer::DrawExApi)
        .addFunction("DrawUI", &Renderer::DrawUIApi)
        .addFunction("DrawUIEx", &Renderer::DrawUIExApi)
        .addFunction("DrawPixel", &Renderer::DrawPixelApi)
        .endNamespace();
}

void Renderer::CameraApi(lua_State *L){
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Camera")
        .addFunction("SetPosition", &Renderer::SetCamPositionApi)
        .addFunction("GetPositionX", &Renderer::GetCamPositionXApi)
        .addFunction("GetPositionY", &Renderer::GetCamPositionYApi)
        .addFunction("SetZoom", &Renderer::SetZoomFactorApi)
        .addFunction("GetZoom", &Renderer::GetZoomFactorApi)
        .endNamespace();
}
void Renderer::SetCamPositionApi(float x, float y){
    Renderer::cam_position.x = x;
    Renderer::cam_position.y = y;
}
float Renderer::GetCamPositionXApi(){
    return Renderer::cam_position.x;
}
float Renderer::GetCamPositionYApi(){
    return Renderer::cam_position.y;
}
void Renderer::SetZoomFactorApi(float zoom_factor_in){
    Renderer::zoom_factor = zoom_factor_in;
    
}
float Renderer::GetZoomFactorApi(){
    return Renderer::zoom_factor;
}
void Renderer::DrawApi(const std::string &image_name, float x, float y){
    draw_actor_queue_draw.push_back(DrawActorRequest {image_name, x, y, 0, 1.0f * zoom_factor, 1.0f * zoom_factor, 0.5f, 0.5f, 255, 255, 255, 255, 0});
}
void Renderer::DrawExApi(const std::string &image_name, float x, float y,float rotate,  float scale_x, float scale_y,float piv_x, float piv_y, float r, float g, float b, float a, int sort_ord){
    draw_actor_queue_draw.push_back(DrawActorRequest {image_name, x, y, rotate, scale_x * zoom_factor, scale_y * zoom_factor, piv_x, piv_y, r, g, b, a, sort_ord});
}
void Renderer::DrawUIApi(const std::string &image_name, float x, float y){
    draw_actor_queue.push_back(DrawActorRequest {image_name, static_cast<float>(static_cast<int>(x)), static_cast<float>(static_cast<int>(y)), 0, 1.0f * zoom_factor, 1.0f * zoom_factor, 0, 0, 255, 255, 255, 255, 0});
}
void Renderer::DrawUIExApi(const std::string &image_name, float x, float y, float r, float g, float b, float a, int sort_ord){
    draw_actor_queue.push_back(DrawActorRequest {image_name, static_cast<float>(static_cast<int>(x)), static_cast<float>(static_cast<int>(y)), 0, 1.0f * zoom_factor, 1.0f * zoom_factor, 0, 0, static_cast<float>(static_cast<int>(r)), static_cast<float>(static_cast<int>(g)), static_cast<float>(static_cast<int>(b)), static_cast<float>(static_cast<int>(a)), sort_ord});
}
void Renderer::DrawPixelApi(float x, float y, float r, float g, float b, float a){
    draw_actor_queue_pixel.push_back(DrawActorRequest {"", (float)(int)x, (float)(int)y,0.0f, 1.0f * zoom_factor, 1.0f * zoom_factor, 0.0f, 0.0f,
        r, g, b, a, 0
    });
}
void Renderer::GamePoll(bool &game_running, GameState &game_state_in) {
    int viewport_w = win_res_x;
    int viewport_h = win_res_y;
    if (render != nullptr) {
        SDL_GetRendererOutputSize(render, &viewport_w, &viewport_h);
    }
    Input::SetViewportSize(viewport_w, viewport_h);

    while (Helper::SDL_PollEvent(&game_render_poll_event)) {
        Input::ProcessEvent(game_render_poll_event);
        switch (game_render_poll_event.type) {
            case SDL_QUIT:
                game_running = false;
                game_state_in = GameState::Quit;
                //RenderFrame();
                return;
            case SDL_APP_WILLENTERBACKGROUND:
                suspended = true;
                break;
            case SDL_APP_DIDENTERFOREGROUND:
                suspended = false;
                break;
            case SDL_KEYDOWN:
            case SDL_MOUSEBUTTONDOWN:
                switch (game_state_in) {
                    case GameState::Intro:
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }
}
void Renderer::RenderFrame() {
    Helper::SDL_RenderPresent(render);
}


void Renderer::SetWindowParameters(std::string &GameTitle_in, int res_x_in, int res_y_in) {
    window_title = GameTitle_in.c_str();
    win_res_x = res_x_in;
    win_res_y = res_y_in;
}
void Renderer::SetClearColor(int red, int blue, int green) {
    clear_red = red;
    clear_blue = blue;
    clear_green = green;
}
void Renderer::ClearWindow() {
    SDL_SetRenderDrawColor(render, clear_red, clear_green, clear_blue, 255);
    SDL_RenderClear(render);
}
void Renderer::StartWindow() {
    window = Helper::SDL_CreateWindow(window_title, 100, 100, win_res_x, win_res_y, SDL_WINDOW_SHOWN);
    render = Helper::SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(render, clear_red, clear_green, clear_blue, 255);
}

void Renderer::RenderImage(SDL_Texture *to_draw, int x, int y, int width, int height) {
    SDL_FRect rect;
    rect.x = x;
    rect.y = y;
    rect.h = height;
    rect.w = width;
    Helper::SDL_RenderCopy(render, to_draw, NULL, &rect);
}

void Renderer::ReserveActorSpace(size_t num_actors) {
    draw_actor_queue.reserve(num_actors);
}


void Renderer::ClearDrawQueue() {
    draw_actor_queue.clear();
}


void ActorRenderer::DrawActor(Actor &actor_in) {
}


void ActorRenderer::RenderActorQueue(Renderer &game_renderer, ImageDB *image_data_base, glm::vec2 cam_offset) {
    std::stable_sort(Renderer::draw_actor_queue.begin(), Renderer::draw_actor_queue.end(), RenderSortingOrder{});
    std::stable_sort(Renderer::draw_actor_queue_draw.begin(), Renderer::draw_actor_queue_draw.end(), RenderSortingOrder{});
    cam_offset = Renderer::cam_position;
    float zoom_factor = Renderer::zoom_factor;
    SDL_RenderSetScale(game_renderer.RenderPtr(), zoom_factor,zoom_factor);
    for(auto &to_draw : Renderer::draw_actor_queue_draw){
        SDL_Texture *img;
        TextureInfo texture_info;
        texture_info = image_data_base->GetTexture(to_draw.image_name);
        img = texture_info.texture;
        to_draw.x -= cam_offset.x;
        to_draw.y -= cam_offset.y;
        to_draw.pivot_x = to_draw.pivot_x * texture_info.width * to_draw.scale_x;
        to_draw.pivot_y = to_draw.pivot_y * texture_info.height * to_draw.scale_y;
        const SDL_FRect rect
                     = { ((to_draw.x * 100.0f) + (game_renderer.win_res_x / (2.0f * zoom_factor))) - to_draw.pivot_x,((to_draw.y * 100.0f) + (game_renderer.win_res_y / (2.0f * zoom_factor))) - to_draw.pivot_y,(glm::abs(to_draw.scale_x) * texture_info.width), (glm::abs(to_draw.scale_y) * texture_info.height) };
                    const SDL_FPoint pivot_pt = { to_draw.pivot_x, to_draw.pivot_y };
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        SDL_SetTextureColorMod(img, to_draw.r, to_draw.g, to_draw.b);
        SDL_SetTextureAlphaMod(img, to_draw.a);
        Helper::SDL_RenderCopyEx(0, "", game_renderer.RenderPtr(), img, NULL, &rect,to_draw.rotation_deg, &pivot_pt, flip);
        SDL_SetTextureColorMod(texture_info.texture, 255, 255, 255);
        SDL_SetTextureAlphaMod(texture_info.texture, 255);
    }
    SDL_RenderSetScale(game_renderer.RenderPtr(), 1.0f,1.0f);
    for(auto &to_draw : Renderer::draw_actor_queue){
        SDL_Texture *img;
        TextureInfo texture_info;
        texture_info = image_data_base->GetTexture(to_draw.image_name);
        img = texture_info.texture;
        const SDL_FRect rect
                     = { (to_draw.x),(to_draw.y),(glm::abs(to_draw.scale_x) * texture_info.width), (glm::abs(to_draw.scale_y) * texture_info.height) };
                    const SDL_FPoint pivot_pt = { to_draw.pivot_x, to_draw.pivot_y };
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        SDL_SetTextureColorMod(img, to_draw.r, to_draw.g, to_draw.b);
        SDL_SetTextureAlphaMod(img, to_draw.a);
        Helper::SDL_RenderCopyEx(0, "", game_renderer.RenderPtr(), img, NULL, &rect,to_draw.rotation_deg, &pivot_pt, flip);
        SDL_SetTextureColorMod(texture_info.texture, 255, 255, 255);
        SDL_SetTextureAlphaMod(texture_info.texture, 255);

    }
    Renderer::draw_actor_queue_draw.clear();
    Renderer::draw_actor_queue.clear();
}
void ActorRenderer::RenderPixelQueue(Renderer &game_renderer, ImageDB *image_data_base, glm::vec2 cam_offset){
    for(auto &to_draw : Renderer::draw_actor_queue_pixel){
        SDL_SetRenderDrawColor(
            game_renderer.RenderPtr(),
            static_cast<Uint8>(to_draw.r ),
            static_cast<Uint8>(to_draw.g),
            static_cast<Uint8>(to_draw.b),
            static_cast<Uint8>(to_draw.a)
        );
        SDL_SetRenderDrawBlendMode(game_renderer.RenderPtr(), SDL_BLENDMODE_BLEND);
        
        SDL_RenderDrawPoint(game_renderer.RenderPtr(), to_draw.x, to_draw.y);
    }
    SDL_SetRenderDrawBlendMode(game_renderer.RenderPtr(), SDL_BLENDMODE_NONE);
    Renderer::draw_actor_queue_pixel.clear();
}
