CXX      := clang++
CC       := clang
PKG_CONF := pkg-config

CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O3
CFLAGS   := -Wall -Wextra -Wpedantic -O3
LDFLAGS  :=

# SDL strategy (mirrors CMake behavior):
#   default: system SDL via pkg-config
#   fallback: bundled SDL folders in this repo
USE_SYSTEM_SDL  ?= 1
USE_BUNDLED_SDL ?= 1

SRC_DIR     := src/FirstParty
BOX2D_DIR   := src/ThirdParty/box2d/src
LUA_DIR     := src/ThirdParty/lua
BUILD_DIR   := build
TARGET      ?= game_engine_linux

COMMON_INCLUDES := \
	-I. \
	-Isrc/FirstParty \
	-Isrc/ThirdParty \
	-Isrc/ThirdParty/sdl_compat \
	-Isrc/ThirdParty/box2d/include \
	-Isrc/ThirdParty/box2d/include/box2d \
	-Isrc/ThirdParty/box2d/src \
	-Isrc/ThirdParty/glm \
	-Isrc/ThirdParty/rapidjson-1.1.0/include \
	-Isrc/ThirdParty/lua \
	-Isrc/ThirdParty/LuaBridge \
	-Isrc/ThirdParty/LuaBridge/detail

INCLUDES := $(COMMON_INCLUDES)
LIBS     :=

# System SDL (default)
ifeq ($(USE_SYSTEM_SDL),1)
SDL_PKG_CFLAGS := $(shell $(PKG_CONF) --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf 2>/dev/null)
SDL_PKG_LIBS   := $(shell $(PKG_CONF) --libs sdl2 SDL2_image SDL2_mixer SDL2_ttf 2>/dev/null)

ifneq ($(strip $(SDL_PKG_LIBS)),)
INCLUDES += $(SDL_PKG_CFLAGS)
LIBS     += $(SDL_PKG_LIBS)
else
ifneq ($(USE_BUNDLED_SDL),1)
$(error Could not resolve SDL2 packages via pkg-config and USE_BUNDLED_SDL=0)
endif
endif
endif

# Bundled SDL fallback
ifeq ($(strip $(LIBS)),)
ifneq ($(USE_BUNDLED_SDL),1)
$(error SDL libs not resolved. Set USE_SYSTEM_SDL=1 or USE_BUNDLED_SDL=1)
endif

INCLUDES += -ISDL2 -ISDL2_img -ISDL2_mix -ISDL2_ttf

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LIBS += \
	./SDL2/lib/SDL2.framework/Versions/A/SDL2 \
	./SDL2_img/lib/SDL2_image.framework/Versions/A/SDL2_image \
	./SDL2_mix/lib/SDL2_mixer.framework/Versions/A/SDL2_mixer \
	./SDL2_ttf/lib/SDL2_ttf.framework/Versions/A/SDL2_ttf
else ifeq ($(OS),Windows_NT)
LIBS += \
	./SDL2/lib/SDL2.lib \
	./SDL2_img/lib/SDL2_image.lib \
	./SDL2_mix/lib/SDL2_mixer.lib \
	./SDL2_ttf/lib/SDL2_ttf.lib
else
$(error Bundled SDL fallback for this OS is not configured. Install system SDL packages.)
endif
endif

CPP_SOURCES := \
	$(wildcard $(SRC_DIR)/*.cpp) \
	$(wildcard $(BOX2D_DIR)/*/*.cpp)

C_SOURCES := $(wildcard $(LUA_DIR)/*.c)

CPP_OBJECTS := \
	$(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(CPP_SOURCES))) \
	$(patsubst $(BOX2D_DIR)/%.cpp,$(BUILD_DIR)/box2d/%.o,$(filter $(BOX2D_DIR)/%.cpp,$(CPP_SOURCES)))

C_OBJECTS := $(patsubst $(LUA_DIR)/%.c,$(BUILD_DIR)/lua/%.o,$(C_SOURCES))

OBJECTS := $(CPP_OBJECTS) $(C_OBJECTS)

.PHONY: all clean print-config

all: $(TARGET)

print-config:
	@echo "USE_SYSTEM_SDL=$(USE_SYSTEM_SDL)"
	@echo "USE_BUNDLED_SDL=$(USE_BUNDLED_SDL)"
	@echo "INCLUDES=$(INCLUDES)"
	@echo "LIBS=$(LIBS)"

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/box2d/%.o: $(BOX2D_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/lua/%.o: $(LUA_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
