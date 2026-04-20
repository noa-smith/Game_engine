//
//  Input.hpp
//  game_engine
//
//  Created by Noah Smith on 2/17/26.
//
#ifndef INPUT_HPP
#define INPUT_HPP

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <array>
#include <optional>
#include <cstdint>

#include "lua.hpp"
#include "LuaBridge.h"

#include "ComponentDB.hpp"
#include "SDL2/SDL.h"
#include "glm/glm.hpp"
#include "AudioHelper.h"
#include "Helper.h"

enum INPUT_STATE {
    KEY_DOWN,
    KEY_JUST_DOWN,
    KEY_UP,
    KEY_JUST_UP,
};

enum class TouchPhase {
    Began,
    Moved,
    Stationary,
    Ended,
    Canceled,
};

struct TouchPoint {
    int64_t finger_id = -1;
    int64_t touch_device_id = -1;
    TouchPhase phase = TouchPhase::Stationary;
    glm::vec2 pixel_pos = glm::vec2(0.0f, 0.0f);
    glm::vec2 normalized_pos = glm::vec2(0.0f, 0.0f);
    glm::vec2 pixel_delta = glm::vec2(0.0f, 0.0f);
    glm::vec2 normalized_delta = glm::vec2(0.0f, 0.0f);
    float pressure = 0.0f;
    bool is_primary = false;
};

class Input {
public:
    static void Init(lua_State *lua_state_);
    static void ProcessEvent(const SDL_Event &e);
    static void LateUpdate();
    static void SetViewportSize(int width, int height);

    static bool GetKey(SDL_Scancode keycode);
    static bool GetKeyUp(SDL_Scancode keycode);
    static bool GetKeyDown(SDL_Scancode keycode);

    static glm::vec2 GetMousePosition();
    
    static bool GetMouseButton(int button);
    static float GetMouseScrollDelta();
    static bool GetMouseDown(uint8_t button);
    static bool GetMouseUp(uint8_t button);

    static int GetTouchCount();
    static std::optional<TouchPoint> GetTouchByIndex(int index);
    static std::optional<TouchPoint> GetTouchById(int64_t finger_id);
    static std::optional<TouchPoint> GetPrimaryTouch();
    static bool GetTouchDown(int64_t finger_id);
    static bool GetTouchUp(int64_t finger_id);
    static bool GetTouchHeld(int64_t finger_id);
    
    static void HideCursor();
    static void ShowCursor();
    
private:
    static inline std::array<INPUT_STATE, SDL_NUM_SCANCODES> key_states;
    static inline std::array<INPUT_STATE, 8> mouse_button_states;
    static inline std::vector<SDL_Scancode> key_just_became_down;
    static inline std::vector<SDL_Scancode> key_just_became_up;
    static inline std::vector<uint8_t> mouse_buttons_just_down;
    static inline std::vector<uint8_t> mouse_buttons_just_up;

    static inline glm::vec2 mouse_position;

    static inline float mouse_scroll_this_frame = 0.0f;

    static inline std::unordered_map<SDL_FingerID, TouchPoint> touch_points;
    static inline std::vector<SDL_FingerID> touch_snapshot_ids;
    static inline std::unordered_set<SDL_FingerID> touch_just_began;
    static inline std::unordered_set<SDL_FingerID> touch_just_ended;
    static inline std::optional<SDL_FingerID> primary_finger_id;

    static inline int viewport_width = 640;
    static inline int viewport_height = 340;

    static inline bool mouse_event_seen_this_frame = false;
    static inline bool mouse_button_event_seen_this_frame = false;
    static inline bool mouse_left_bridged_active = false;

    static void RebuildTouchSnapshot();
    static bool IsTouchActive(const TouchPoint &touch);
    static void SelectPrimaryTouchIfNeeded();
    static void BridgePrimaryTouchToMouse();
    static std::optional<TouchPoint> LookupTouch(int64_t finger_id);
};
#endif
