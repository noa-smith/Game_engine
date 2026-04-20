//
//  Renderer.hpp
//  game_engine
//
//  Created by Noah Smith on 2/4/26.
//
#ifndef Renderer_hpp
#define Renderer_hpp

// #include "AudioHelper.h"
#include "Actor.hpp"
#include "EngineUtil.h"
#include "ImageDB.hpp"
#include "textDB.hpp"
struct DrawActorRequest {
    std::string image_name;
    float x;
    float y;
    float rotation_deg = 0.0f;
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    float pivot_x = 0.0f;
    float pivot_y = 0.0f;
    float r = 255.0f;
    float g = 255.0f;
    float b = 255.0f;
    float a = 255.0f;
    int sort_ord;
};

class Renderer {
public:
    void SetWindowParameters(std::string &title, int res_x_in, int res_y_in);
    void SetClearColor(int red, int blue, int green);
    void ClearWindow();
    void StartRenderer();
    void RenderFrame();
    void GamePoll(bool &game_running, GameState &game_state_in);
    bool IsSuspended() const { return suspended; }
    SDL_Renderer *RenderPtr();
    void RenderImage(SDL_Texture *to_draw, int x, int y, int width, int height);
    void DrawActors(ImageDB &img_db, std::vector<std::unique_ptr<Actor>> &actors);
    void ClearDrawQueue();
    void ReserveActorSpace(size_t num_actors);
    int win_res_x = 640;
    int win_res_y = 340;
    static inline float zoom_factor = 1.0f;
    static inline glm::vec2 cam_position = ZERO;
    static void SetImageDB(ImageDB * img_db);
    inline static ImageDB *image_data_base_ptr;
    void ImageApi(lua_State *L);
    void CameraApi(lua_State *L);
    static inline std::vector<DrawActorRequest> draw_actor_queue;
    static inline std::vector<DrawActorRequest> draw_actor_queue_draw;
    static inline std::vector<DrawActorRequest> draw_actor_queue_ui;
    static inline std::vector<DrawActorRequest> draw_actor_queue_pixel;
    static void SetCamPositionApi(float x, float y);
    static float GetCamPositionXApi();
    static float GetCamPositionYApi();
    static void SetZoomFactorApi(float zoom_factor_in);
    static float GetZoomFactorApi();
    static void DrawApi(const std::string &image_name, float x, float y);
    static void DrawExApi(const std::string &image_name, float x, float y,float rotate,  float scale_x, float scale_y,float piv_x, float piv_y, float r, float g, float b, float a, int sort_ord);
    static void DrawUIApi(const std::string &image_name, float x, float y);
    static void DrawUIExApi(const std::string &image_name, float x, float y, float r, float g, float b, float a, int sort_ord);
    static void DrawPixelApi(float x, float y, float r, float g, float b, float a);
    ~Renderer();
    
private:
    void StartWindow();
    void HandleIntroEvent(SDL_Event &e);
    int intro_index = 0;
    SDL_Window *window = nullptr;
    SDL_Renderer *render = nullptr;
    SDL_Event game_render_poll_event;
    const char *window_title;
    
    
    bool suspended = false;
    uint8_t clear_red = 255;
    uint8_t clear_blue = 255;
    uint8_t clear_green = 255;
};
struct RenderSortingOrder {
    bool operator()(const DrawActorRequest &a, const DrawActorRequest &b) const { return a.sort_ord < b.sort_ord;}
};

namespace ActorRenderer {
    void DrawActor(Actor &actor_in);
    void RenderActorQueue(Renderer &game_renderer, ImageDB* image_data_base, glm::vec2 cam_offset);
    void RenderPixelQueue(Renderer &game_renderer, ImageDB *image_data_base, glm::vec2 cam_offset);
}

#endif
