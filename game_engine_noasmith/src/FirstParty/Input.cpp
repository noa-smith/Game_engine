//
//  Input.cpp
//  game_engine
//
//  Created by Noah Smith on 2/17/26.
//
#include "Input.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace {

lua_State* g_input_lua_state = nullptr;

const char* TouchPhaseToString(TouchPhase phase) {
    switch (phase) {
        case TouchPhase::Began: return "Began";
        case TouchPhase::Moved: return "Moved";
        case TouchPhase::Stationary: return "Stationary";
        case TouchPhase::Ended: return "Ended";
        case TouchPhase::Canceled: return "Canceled";
        default: return "Stationary";
    }
}

bool IsMouseEventType(Uint32 type) {
    return type == SDL_MOUSEBUTTONDOWN || type == SDL_MOUSEBUTTONUP || type == SDL_MOUSEMOTION || type == SDL_MOUSEWHEEL;
}

bool IsTouchEventType(Uint32 type) {
    if (type == SDL_FINGERDOWN || type == SDL_FINGERMOTION || type == SDL_FINGERUP) {
        return true;
    }
#if defined(SDL_FINGERCANCEL)
    if (type == SDL_FINGERCANCEL) {
        return true;
    }
#endif
    return false;
}

bool IsLeftMouseDownState(INPUT_STATE state) {
    return state == INPUT_STATE::KEY_DOWN || state == INPUT_STATE::KEY_JUST_DOWN;
}

luabridge::LuaRef BuildTouchLuaRef(const std::optional<TouchPoint>& touch) {
    if (!g_input_lua_state || !touch.has_value()) {
        return luabridge::LuaRef(g_input_lua_state);
    }
    luabridge::LuaRef out = luabridge::newTable(g_input_lua_state);
    out["id"] = static_cast<lua_Integer>(touch->finger_id);
    out["device_id"] = static_cast<lua_Integer>(touch->touch_device_id);
    out["phase"] = std::string(TouchPhaseToString(touch->phase));
    out["x"] = touch->pixel_pos.x;
    out["y"] = touch->pixel_pos.y;
    out["nx"] = touch->normalized_pos.x;
    out["ny"] = touch->normalized_pos.y;
    out["dx"] = touch->pixel_delta.x;
    out["dy"] = touch->pixel_delta.y;
    out["ndx"] = touch->normalized_delta.x;
    out["ndy"] = touch->normalized_delta.y;
    out["pressure"] = touch->pressure;
    out["primary"] = touch->is_primary;
    return out;
}

int LuaGetTouchCount() {
    return Input::GetTouchCount();
}

luabridge::LuaRef LuaGetTouch(int lua_index) {
    return BuildTouchLuaRef(Input::GetTouchByIndex(lua_index - 1));
}

luabridge::LuaRef LuaGetPrimaryTouch() {
    return BuildTouchLuaRef(Input::GetPrimaryTouch());
}

bool LuaGetTouchDown(lua_Integer finger_id) {
    return Input::GetTouchDown(static_cast<int64_t>(finger_id));
}

bool LuaGetTouchUp(lua_Integer finger_id) {
    return Input::GetTouchUp(static_cast<int64_t>(finger_id));
}

bool LuaGetTouchHeld(lua_Integer finger_id) {
    return Input::GetTouchHeld(static_cast<int64_t>(finger_id));
}

} // namespace

void Input::Init(lua_State *lua_state_) {
    g_input_lua_state = lua_state_;
    key_states.fill(INPUT_STATE::KEY_UP);
    mouse_button_states.fill(INPUT_STATE::KEY_UP);

    touch_points.clear();
    touch_snapshot_ids.clear();
    touch_just_began.clear();
    touch_just_ended.clear();
    primary_finger_id.reset();
    mouse_left_bridged_active = false;
    mouse_event_seen_this_frame = false;
    mouse_button_event_seen_this_frame = false;

    luabridge::getGlobalNamespace(lua_state_)
        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass();

    luabridge::getGlobalNamespace(lua_state_)
        .beginNamespace("Input")
        .addFunction("GetKey", +[](const std::string &scancode) {
            auto key = __keycode_to_scancode.find(scancode);
            if (key != __keycode_to_scancode.end()) {
                return GetKey(key->second);
            }
            return false;
        })
        .addFunction("GetKeyDown", +[](const std::string &scancode) {
            auto key = __keycode_to_scancode.find(scancode);
            if (key != __keycode_to_scancode.end()) {
                return GetKeyDown(key->second);
            }
            return false;
        })
        .addFunction("GetKeyUp", +[](const std::string &scancode) {
            auto key = __keycode_to_scancode.find(scancode);
            if (key != __keycode_to_scancode.end()) {
                return GetKeyUp(key->second);
            }
            return false;
        })
        .addFunction("GetMousePosition", GetMousePosition)
        .addFunction("GetMouseButton", +[](const int &button) {
            return GetMouseButton(static_cast<uint8_t>(button));
        })
        .addFunction("GetMouseButtonDown", +[](const int &button) {
            return GetMouseDown(button);
        })
        .addFunction("GetMouseButtonUp", +[](const int &button) {
            return GetMouseUp(button);
        })
        .addFunction("GetMouseScrollDelta", GetMouseScrollDelta)
        .addFunction("GetTouchCount", &LuaGetTouchCount)
        .addFunction("GetTouch", &LuaGetTouch)
        .addFunction("GetPrimaryTouch", &LuaGetPrimaryTouch)
        .addFunction("GetTouchDown", &LuaGetTouchDown)
        .addFunction("GetTouchUp", &LuaGetTouchUp)
        .addFunction("GetTouchHeld", &LuaGetTouchHeld)
        .addFunction("HideCursor", HideCursor)
        .addFunction("ShowCursor", ShowCursor)
        .endNamespace();
}

void Input::SetViewportSize(int width, int height) {
    viewport_width = std::max(1, width);
    viewport_height = std::max(1, height);
}

void Input::ProcessEvent(const SDL_Event &e) {
    if (IsMouseEventType(e.type)) {
        mouse_event_seen_this_frame = true;
        if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            mouse_button_event_seen_this_frame = true;
            if (e.button.button == SDL_BUTTON_LEFT) {
                mouse_left_bridged_active = false;
            }
        } else if (mouse_left_bridged_active && !mouse_button_event_seen_this_frame) {
            INPUT_STATE &state = mouse_button_states[SDL_BUTTON_LEFT];
            if (IsLeftMouseDownState(state)) {
                state = INPUT_STATE::KEY_JUST_UP;
                mouse_buttons_just_up.push_back(SDL_BUTTON_LEFT);
            }
            mouse_left_bridged_active = false;
        }
    }

    if (e.type != SDL_KEYDOWN && e.type != SDL_KEYUP &&
        e.type != SDL_MOUSEBUTTONDOWN && e.type != SDL_MOUSEBUTTONUP &&
        e.type != SDL_MOUSEMOTION && e.type != SDL_MOUSEWHEEL &&
        !IsTouchEventType(e.type)) {
        return;
    }

    switch (e.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            const SDL_Scancode code = e.key.keysym.scancode;
            INPUT_STATE &state = key_states[code];
            if (e.type == SDL_KEYDOWN) {
                if (state == INPUT_STATE::KEY_UP || state == INPUT_STATE::KEY_JUST_UP) {
                    state = INPUT_STATE::KEY_JUST_DOWN;
                    key_just_became_down.push_back(code);
                }
            } else {
                if (state == INPUT_STATE::KEY_DOWN || state == INPUT_STATE::KEY_JUST_DOWN) {
                    state = INPUT_STATE::KEY_JUST_UP;
                    key_just_became_up.push_back(code);
                }
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            const uint8_t button = e.button.button;
            INPUT_STATE &state = mouse_button_states[button];
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (state == INPUT_STATE::KEY_UP || state == INPUT_STATE::KEY_JUST_UP) {
                    state = INPUT_STATE::KEY_JUST_DOWN;
                    mouse_buttons_just_down.push_back(button);
                }
            } else {
                if (state == INPUT_STATE::KEY_DOWN || state == INPUT_STATE::KEY_JUST_DOWN) {
                    state = INPUT_STATE::KEY_JUST_UP;
                    mouse_buttons_just_up.push_back(button);
                }
            }
            break;
        }
        case SDL_MOUSEMOTION:
            mouse_position.x = static_cast<float>(e.motion.x);
            mouse_position.y = static_cast<float>(e.motion.y);
            break;
        case SDL_MOUSEWHEEL:
            mouse_scroll_this_frame = e.wheel.preciseY;
            break;
        case SDL_FINGERDOWN: {
            const SDL_FingerID finger_id = e.tfinger.fingerId;
            TouchPoint &touch = touch_points[finger_id];
            touch.finger_id = static_cast<int64_t>(finger_id);
            touch.touch_device_id = static_cast<int64_t>(e.tfinger.touchId);
            touch.phase = TouchPhase::Began;
            touch.normalized_pos = glm::vec2(e.tfinger.x, e.tfinger.y);
            touch.normalized_delta = glm::vec2(0.0f, 0.0f);
            touch.pixel_pos = glm::vec2(e.tfinger.x * static_cast<float>(viewport_width), e.tfinger.y * static_cast<float>(viewport_height));
            touch.pixel_delta = glm::vec2(0.0f, 0.0f);
            touch.pressure = e.tfinger.pressure;
            touch_just_began.insert(finger_id);
            touch_just_ended.erase(finger_id);
            if (!primary_finger_id.has_value()) {
                primary_finger_id = finger_id;
            }
            SelectPrimaryTouchIfNeeded();
            RebuildTouchSnapshot();
            BridgePrimaryTouchToMouse();
            break;
        }
        case SDL_FINGERMOTION: {
            const SDL_FingerID finger_id = e.tfinger.fingerId;
            TouchPoint &touch = touch_points[finger_id];
            touch.finger_id = static_cast<int64_t>(finger_id);
            touch.touch_device_id = static_cast<int64_t>(e.tfinger.touchId);
            touch.phase = TouchPhase::Moved;
            touch.normalized_pos = glm::vec2(e.tfinger.x, e.tfinger.y);
            touch.normalized_delta = glm::vec2(e.tfinger.dx, e.tfinger.dy);
            touch.pixel_pos = glm::vec2(e.tfinger.x * static_cast<float>(viewport_width), e.tfinger.y * static_cast<float>(viewport_height));
            touch.pixel_delta = glm::vec2(e.tfinger.dx * static_cast<float>(viewport_width), e.tfinger.dy * static_cast<float>(viewport_height));
            touch.pressure = e.tfinger.pressure;
            touch_just_ended.erase(finger_id);
            SelectPrimaryTouchIfNeeded();
            RebuildTouchSnapshot();
            BridgePrimaryTouchToMouse();
            break;
        }
        case SDL_FINGERUP: {
            const SDL_FingerID finger_id = e.tfinger.fingerId;
            TouchPoint &touch = touch_points[finger_id];
            touch.finger_id = static_cast<int64_t>(finger_id);
            touch.touch_device_id = static_cast<int64_t>(e.tfinger.touchId);
            touch.phase = TouchPhase::Ended;
            touch.normalized_pos = glm::vec2(e.tfinger.x, e.tfinger.y);
            touch.normalized_delta = glm::vec2(e.tfinger.dx, e.tfinger.dy);
            touch.pixel_pos = glm::vec2(e.tfinger.x * static_cast<float>(viewport_width), e.tfinger.y * static_cast<float>(viewport_height));
            touch.pixel_delta = glm::vec2(e.tfinger.dx * static_cast<float>(viewport_width), e.tfinger.dy * static_cast<float>(viewport_height));
            touch.pressure = e.tfinger.pressure;
            touch_just_began.erase(finger_id);
            touch_just_ended.insert(finger_id);
            SelectPrimaryTouchIfNeeded();
            RebuildTouchSnapshot();
            BridgePrimaryTouchToMouse();
            break;
        }
#if defined(SDL_FINGERCANCEL)
        case SDL_FINGERCANCEL: {
            const SDL_FingerID finger_id = e.tfinger.fingerId;
            TouchPoint &touch = touch_points[finger_id];
            touch.finger_id = static_cast<int64_t>(finger_id);
            touch.touch_device_id = static_cast<int64_t>(e.tfinger.touchId);
            touch.phase = TouchPhase::Canceled;
            touch.normalized_pos = glm::vec2(e.tfinger.x, e.tfinger.y);
            touch.normalized_delta = glm::vec2(e.tfinger.dx, e.tfinger.dy);
            touch.pixel_pos = glm::vec2(e.tfinger.x * static_cast<float>(viewport_width), e.tfinger.y * static_cast<float>(viewport_height));
            touch.pixel_delta = glm::vec2(e.tfinger.dx * static_cast<float>(viewport_width), e.tfinger.dy * static_cast<float>(viewport_height));
            touch.pressure = e.tfinger.pressure;
            touch_just_began.erase(finger_id);
            touch_just_ended.insert(finger_id);
            SelectPrimaryTouchIfNeeded();
            RebuildTouchSnapshot();
            BridgePrimaryTouchToMouse();
            break;
        }
#endif
        default:
            break;
    }
}

void Input::LateUpdate() {
    for (const SDL_Scancode code : key_just_became_down) {
        key_states[code] = INPUT_STATE::KEY_DOWN;
    }
    for (const SDL_Scancode code : key_just_became_up) {
        key_states[code] = INPUT_STATE::KEY_UP;
    }
    key_just_became_down.clear();
    key_just_became_up.clear();

    for (const uint8_t b : mouse_buttons_just_down) {
        mouse_button_states[b] = INPUT_STATE::KEY_DOWN;
    }
    for (const uint8_t b : mouse_buttons_just_up) {
        mouse_button_states[b] = INPUT_STATE::KEY_UP;
    }
    mouse_buttons_just_up.clear();
    mouse_buttons_just_down.clear();

    std::vector<SDL_FingerID> to_remove;
    to_remove.reserve(touch_points.size());
    for (auto &[id, touch] : touch_points) {
        if (touch.phase == TouchPhase::Began || touch.phase == TouchPhase::Moved) {
            touch.phase = TouchPhase::Stationary;
            touch.normalized_delta = glm::vec2(0.0f, 0.0f);
            touch.pixel_delta = glm::vec2(0.0f, 0.0f);
        } else if (touch.phase == TouchPhase::Ended || touch.phase == TouchPhase::Canceled) {
            to_remove.push_back(id);
        } else {
            touch.normalized_delta = glm::vec2(0.0f, 0.0f);
            touch.pixel_delta = glm::vec2(0.0f, 0.0f);
        }
    }
    for (SDL_FingerID id : to_remove) {
        touch_points.erase(id);
    }
    touch_just_began.clear();
    touch_just_ended.clear();
    SelectPrimaryTouchIfNeeded();
    RebuildTouchSnapshot();

    mouse_event_seen_this_frame = false;
    mouse_button_event_seen_this_frame = false;
    mouse_scroll_this_frame = 0.0f;
}

bool Input::GetKey(SDL_Scancode keycode) {
    return key_states[keycode] == INPUT_STATE::KEY_DOWN || key_states[keycode] == INPUT_STATE::KEY_JUST_DOWN;
}

bool Input::GetKeyUp(SDL_Scancode keycode) {
    return key_states[keycode] == INPUT_STATE::KEY_JUST_UP;
}

bool Input::GetKeyDown(SDL_Scancode keycode) {
    return key_states[keycode] == INPUT_STATE::KEY_JUST_DOWN;
}

bool Input::GetMouseDown(uint8_t button) {
    return mouse_button_states[button] == INPUT_STATE::KEY_JUST_DOWN;
}

bool Input::GetMouseButton(int button) {
    return mouse_button_states[button] == INPUT_STATE::KEY_JUST_DOWN || mouse_button_states[button] == INPUT_STATE::KEY_DOWN;
}

bool Input::GetMouseUp(uint8_t button) {
    return mouse_button_states[button] == INPUT_STATE::KEY_JUST_UP;
}

glm::vec2 Input::GetMousePosition() {
    return mouse_position;
}

float Input::GetMouseScrollDelta() {
    return mouse_scroll_this_frame;
}

int Input::GetTouchCount() {
    return static_cast<int>(touch_snapshot_ids.size());
}

std::optional<TouchPoint> Input::GetTouchByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(touch_snapshot_ids.size())) {
        return std::nullopt;
    }
    const SDL_FingerID id = touch_snapshot_ids[index];
    auto found = touch_points.find(id);
    if (found == touch_points.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::optional<TouchPoint> Input::LookupTouch(int64_t finger_id) {
    auto found = touch_points.find(static_cast<SDL_FingerID>(finger_id));
    if (found == touch_points.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::optional<TouchPoint> Input::GetTouchById(int64_t finger_id) {
    return LookupTouch(finger_id);
}

std::optional<TouchPoint> Input::GetPrimaryTouch() {
    if (!primary_finger_id.has_value()) {
        return std::nullopt;
    }
    auto found = touch_points.find(*primary_finger_id);
    if (found == touch_points.end()) {
        return std::nullopt;
    }
    return found->second;
}

bool Input::GetTouchDown(int64_t finger_id) {
    return touch_just_began.find(static_cast<SDL_FingerID>(finger_id)) != touch_just_began.end();
}

bool Input::GetTouchUp(int64_t finger_id) {
    return touch_just_ended.find(static_cast<SDL_FingerID>(finger_id)) != touch_just_ended.end();
}

bool Input::GetTouchHeld(int64_t finger_id) {
    auto touch = LookupTouch(finger_id);
    return touch.has_value() && IsTouchActive(*touch);
}

void Input::HideCursor() {
    SDL_ShowCursor(SDL_DISABLE);
}

void Input::ShowCursor() {
    SDL_ShowCursor(SDL_ENABLE);
}

void Input::RebuildTouchSnapshot() {
    touch_snapshot_ids.clear();
    touch_snapshot_ids.reserve(touch_points.size());
    for (const auto &[id, touch] : touch_points) {
        (void)touch;
        touch_snapshot_ids.push_back(id);
    }
    std::sort(touch_snapshot_ids.begin(), touch_snapshot_ids.end(), [](SDL_FingerID a, SDL_FingerID b) {
        return static_cast<long long>(a) < static_cast<long long>(b);
    });
    for (auto &[id, touch] : touch_points) {
        touch.is_primary = primary_finger_id.has_value() && *primary_finger_id == id;
    }
}

bool Input::IsTouchActive(const TouchPoint &touch) {
    return touch.phase == TouchPhase::Began || touch.phase == TouchPhase::Moved || touch.phase == TouchPhase::Stationary;
}

void Input::SelectPrimaryTouchIfNeeded() {
    if (primary_finger_id.has_value()) {
        auto it = touch_points.find(*primary_finger_id);
        if (it != touch_points.end() && IsTouchActive(it->second)) {
            return;
        }
    }
    primary_finger_id.reset();
    SDL_FingerID selected_id = 0;
    bool found = false;
    for (const auto &[id, touch] : touch_points) {
        if (!IsTouchActive(touch)) {
            continue;
        }
        if (!found || static_cast<long long>(id) < static_cast<long long>(selected_id)) {
            selected_id = id;
            found = true;
        }
    }
    if (found) {
        primary_finger_id = selected_id;
    }
}

void Input::BridgePrimaryTouchToMouse() {
    if (mouse_event_seen_this_frame) {
        return;
    }
    const auto primary = GetPrimaryTouch();
    const bool pressed = primary.has_value() && IsTouchActive(*primary);
    if (primary.has_value()) {
        mouse_position = primary->pixel_pos;
    }

    INPUT_STATE &left_state = mouse_button_states[SDL_BUTTON_LEFT];
    if (pressed && !IsLeftMouseDownState(left_state)) {
        left_state = INPUT_STATE::KEY_JUST_DOWN;
        mouse_buttons_just_down.push_back(SDL_BUTTON_LEFT);
    } else if (!pressed && IsLeftMouseDownState(left_state)) {
        left_state = INPUT_STATE::KEY_JUST_UP;
        mouse_buttons_just_up.push_back(SDL_BUTTON_LEFT);
    }
    mouse_left_bridged_active = pressed;
}
