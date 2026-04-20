cmake_minimum_required(VERSION 3.10.2)

project(game_engine LANGUAGES C CXX)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/FirstParty")
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty")

set(ENGINE_SOURCES
    ${SOURCE_DIR}/Actor.cpp
    ${SOURCE_DIR}/AndroidPlatformServices.cpp
    ${SOURCE_DIR}/AudioDB.cpp
    ${SOURCE_DIR}/ComponentDB.cpp
    ${SOURCE_DIR}/ContactListener.cpp
    ${SOURCE_DIR}/DesktopPlatformServices.cpp
    ${SOURCE_DIR}/Engine.cpp
    ${SOURCE_DIR}/ImageDB.cpp
    ${SOURCE_DIR}/Input.cpp
    ${SOURCE_DIR}/Physics.cpp
    ${SOURCE_DIR}/PlatformContext.cpp
    ${SOURCE_DIR}/Renderer.cpp
    ${SOURCE_DIR}/RigidBody2D.cpp
    ${SOURCE_DIR}/Scene.cpp
    ${SOURCE_DIR}/Template.cpp
    ${SOURCE_DIR}/textDB.cpp
)

file(GLOB BOX2D_SOURCES
    "${THIRD_PARTY_DIR}/box2d/src/*/*.cpp"
)

set(LUA_SOURCES
    ${THIRD_PARTY_DIR}/lua/lapi.c
    ${THIRD_PARTY_DIR}/lua/lauxlib.c
    ${THIRD_PARTY_DIR}/lua/lbaselib.c
    ${THIRD_PARTY_DIR}/lua/lcode.c
    ${THIRD_PARTY_DIR}/lua/lcorolib.c
    ${THIRD_PARTY_DIR}/lua/lctype.c
    ${THIRD_PARTY_DIR}/lua/ldblib.c
    ${THIRD_PARTY_DIR}/lua/ldebug.c
    ${THIRD_PARTY_DIR}/lua/ldo.c
    ${THIRD_PARTY_DIR}/lua/ldump.c
    ${THIRD_PARTY_DIR}/lua/lfunc.c
    ${THIRD_PARTY_DIR}/lua/lgc.c
    ${THIRD_PARTY_DIR}/lua/linit.c
    ${THIRD_PARTY_DIR}/lua/liolib.c
    ${THIRD_PARTY_DIR}/lua/llex.c
    ${THIRD_PARTY_DIR}/lua/lmathlib.c
    ${THIRD_PARTY_DIR}/lua/lmem.c
    ${THIRD_PARTY_DIR}/lua/loadlib.c
    ${THIRD_PARTY_DIR}/lua/lobject.c
    ${THIRD_PARTY_DIR}/lua/lopcodes.c
    ${THIRD_PARTY_DIR}/lua/loslib.c
    ${THIRD_PARTY_DIR}/lua/lparser.c
    ${THIRD_PARTY_DIR}/lua/lstate.c
    ${THIRD_PARTY_DIR}/lua/lstring.c
    ${THIRD_PARTY_DIR}/lua/lstrlib.c
    ${THIRD_PARTY_DIR}/lua/ltable.c
    ${THIRD_PARTY_DIR}/lua/ltablib.c
    ${THIRD_PARTY_DIR}/lua/ltm.c
    ${THIRD_PARTY_DIR}/lua/lundump.c
    ${THIRD_PARTY_DIR}/lua/lutf8lib.c
    ${THIRD_PARTY_DIR}/lua/lvm.c
    ${THIRD_PARTY_DIR}/lua/lzio.c
)

set(ALL_SOURCES
    ${ENGINE_SOURCES}
    ${BOX2D_SOURCES}
    ${LUA_SOURCES}
)

if(ANDROID)
    add_library(main SHARED ${ALL_SOURCES})
    set(ENGINE_TARGET main)
    target_compile_definitions(main PRIVATE __ANDROID__)
else()
    add_executable(game_engine ${ALL_SOURCES})
    set(ENGINE_TARGET game_engine)
endif()

target_compile_features(${ENGINE_TARGET} PRIVATE cxx_std_17)

# Keep SDL include compatibility wrappers first so either system or bundled
# extension headers are accepted by source includes.
target_include_directories(${ENGINE_TARGET} PRIVATE
    ${THIRD_PARTY_DIR}/sdl_compat
    ${SOURCE_DIR}
    ${THIRD_PARTY_DIR}
    ${THIRD_PARTY_DIR}/box2d/include
    ${THIRD_PARTY_DIR}/box2d/include/box2d
    ${THIRD_PARTY_DIR}/box2d/src
    ${THIRD_PARTY_DIR}/glm
    ${THIRD_PARTY_DIR}/lua
    ${THIRD_PARTY_DIR}/LuaBridge
    ${THIRD_PARTY_DIR}/LuaBridge/detail
    ${THIRD_PARTY_DIR}/rapidjson-1.1.0/include
)

# Android uses repo-provided SDL headers.
if(ANDROID)
    target_include_directories(${ENGINE_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL2
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL2_img
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL2_mix
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf
    )
endif()

option(GAMEENGINE_USE_SYSTEM_SDL "Use installed SDL2/SDL2_image/SDL2_mixer/SDL2_ttf when available" ON)
option(GAMEENGINE_ALLOW_BUNDLED_SDL "Allow fallback to bundled SDL folders in this repo" ON)

set(SDL2_LINK_TARGET "")
set(SDL2_IMAGE_LINK_TARGET "")
set(SDL2_MIXER_LINK_TARGET "")
set(SDL2_TTF_LINK_TARGET "")

function(select_existing_target out_var)
    foreach(candidate IN LISTS ARGN)
        if(TARGET ${candidate})
            set(${out_var} ${candidate} PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${out_var} "" PARENT_SCOPE)
endfunction()

function(collect_sdl_include_dirs out_var)
    set(collected "")
    foreach(dep_target IN LISTS ARGN)
        if(TARGET ${dep_target})
            get_target_property(target_includes ${dep_target} INTERFACE_INCLUDE_DIRECTORIES)
            if(target_includes)
                foreach(include_dir IN LISTS target_includes)
                    if(IS_DIRECTORY "${include_dir}")
                        list(APPEND collected "${include_dir}")
                        get_filename_component(include_leaf "${include_dir}" NAME)
                        if(include_leaf STREQUAL "SDL2")
                            get_filename_component(include_parent "${include_dir}" DIRECTORY)
                            list(APPEND collected "${include_parent}")
                        endif()
                    endif()
                endforeach()
            endif()
        endif()
    endforeach()
    list(REMOVE_DUPLICATES collected)
    set(${out_var} "${collected}" PARENT_SCOPE)
endfunction()

if(ANDROID)
    # Android uses prebuilts by ABI from the SDL android-project tree.
    set(ANDROID_SDL_PREBUILT_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/android-project/app/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}")

    set(SDL2_SO_PATH "${ANDROID_SDL_PREBUILT_DIR}/libSDL2.so")
    set(SDL2_IMAGE_SO_PATH "${ANDROID_SDL_PREBUILT_DIR}/libSDL2_image.so")
    set(SDL2_MIXER_SO_PATH "${ANDROID_SDL_PREBUILT_DIR}/libSDL2_mixer.so")
    set(SDL2_TTF_SO_PATH "${ANDROID_SDL_PREBUILT_DIR}/libSDL2_ttf.so")

    if(NOT EXISTS "${SDL2_SO_PATH}")
        message(FATAL_ERROR "Missing prebuilt SDL2: ${SDL2_SO_PATH}")
    endif()
    if(NOT EXISTS "${SDL2_IMAGE_SO_PATH}")
        message(FATAL_ERROR "Missing prebuilt SDL2_image: ${SDL2_IMAGE_SO_PATH}")
    endif()
    if(NOT EXISTS "${SDL2_MIXER_SO_PATH}")
        message(FATAL_ERROR "Missing prebuilt SDL2_mixer: ${SDL2_MIXER_SO_PATH}")
    endif()
    if(NOT EXISTS "${SDL2_TTF_SO_PATH}")
        message(FATAL_ERROR "Missing prebuilt SDL2_ttf: ${SDL2_TTF_SO_PATH}")
    endif()

    add_library(SDL2_IMPORTED SHARED IMPORTED)
    set_target_properties(SDL2_IMPORTED PROPERTIES IMPORTED_LOCATION "${SDL2_SO_PATH}")

    add_library(SDL2_IMAGE_IMPORTED SHARED IMPORTED)
    set_target_properties(SDL2_IMAGE_IMPORTED PROPERTIES IMPORTED_LOCATION "${SDL2_IMAGE_SO_PATH}")

    add_library(SDL2_MIXER_IMPORTED SHARED IMPORTED)
    set_target_properties(SDL2_MIXER_IMPORTED PROPERTIES IMPORTED_LOCATION "${SDL2_MIXER_SO_PATH}")

    add_library(SDL2_TTF_IMPORTED SHARED IMPORTED)
    set_target_properties(SDL2_TTF_IMPORTED PROPERTIES IMPORTED_LOCATION "${SDL2_TTF_SO_PATH}")

    set(SDL2_LINK_TARGET SDL2_IMPORTED)
    set(SDL2_IMAGE_LINK_TARGET SDL2_IMAGE_IMPORTED)
    set(SDL2_MIXER_LINK_TARGET SDL2_MIXER_IMPORTED)
    set(SDL2_TTF_LINK_TARGET SDL2_TTF_IMPORTED)
else()
    if(GAMEENGINE_USE_SYSTEM_SDL)
        find_package(SDL2 QUIET CONFIG)
        if(NOT SDL2_FOUND)
            find_package(SDL2 QUIET)
        endif()

        find_package(SDL2_image QUIET CONFIG)
        find_package(SDL2_mixer QUIET CONFIG)
        find_package(SDL2_ttf QUIET CONFIG)

        select_existing_target(SDL2_LINK_TARGET
            SDL2::SDL2
            SDL2::SDL2-shared
            SDL2::SDL2-static
        )

        select_existing_target(SDL2_IMAGE_LINK_TARGET
            SDL2_image::SDL2_image
            SDL2_image::SDL2_image-shared
            SDL2_image::SDL2_image-static
        )

        select_existing_target(SDL2_MIXER_LINK_TARGET
            SDL2_mixer::SDL2_mixer
            SDL2_mixer::SDL2_mixer-shared
            SDL2_mixer::SDL2_mixer-static
        )

        select_existing_target(SDL2_TTF_LINK_TARGET
            SDL2_ttf::SDL2_ttf
            SDL2_ttf::SDL2_ttf-shared
            SDL2_ttf::SDL2_ttf-static
        )

        if(SDL2_LINK_TARGET AND SDL2_IMAGE_LINK_TARGET AND SDL2_MIXER_LINK_TARGET AND SDL2_TTF_LINK_TARGET)
            message(STATUS "Using system SDL dependencies")
            collect_sdl_include_dirs(SYSTEM_SDL_INCLUDE_DIRS
                ${SDL2_LINK_TARGET}
                ${SDL2_IMAGE_LINK_TARGET}
                ${SDL2_MIXER_LINK_TARGET}
                ${SDL2_TTF_LINK_TARGET}
            )
            if(SYSTEM_SDL_INCLUDE_DIRS)
                target_include_directories(${ENGINE_TARGET} PRIVATE ${SYSTEM_SDL_INCLUDE_DIRS})
            endif()
        else()
            set(SDL2_LINK_TARGET "")
            set(SDL2_IMAGE_LINK_TARGET "")
            set(SDL2_MIXER_LINK_TARGET "")
            set(SDL2_TTF_LINK_TARGET "")
            message(STATUS "System SDL packages were not fully resolved; trying bundled SDL")
        endif()
    endif()

    if(NOT SDL2_LINK_TARGET)
        if(NOT GAMEENGINE_ALLOW_BUNDLED_SDL)
            message(FATAL_ERROR
                "SDL dependencies not resolved via system packages and bundled fallback is disabled. "
                "Install SDL2 + SDL2_image + SDL2_mixer + SDL2_ttf or set -DGAMEENGINE_ALLOW_BUNDLED_SDL=ON.")
        endif()

        target_include_directories(${ENGINE_TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/SDL2
            ${CMAKE_CURRENT_SOURCE_DIR}/SDL2_img
            ${CMAKE_CURRENT_SOURCE_DIR}/SDL2_mix
            ${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf
        )

        if(APPLE
           AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/SDL2.framework/Versions/A/SDL2"
           AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_img/lib/SDL2_image.framework/Versions/A/SDL2_image"
           AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_mix/lib/SDL2_mixer.framework/Versions/A/SDL2_mixer"
           AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf/lib/SDL2_ttf.framework/Versions/A/SDL2_ttf")
            set(SDL2_LINK_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/SDL2.framework/Versions/A/SDL2")
            set(SDL2_IMAGE_LINK_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_img/lib/SDL2_image.framework/Versions/A/SDL2_image")
            set(SDL2_MIXER_LINK_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_mix/lib/SDL2_mixer.framework/Versions/A/SDL2_mixer")
            set(SDL2_TTF_LINK_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf/lib/SDL2_ttf.framework/Versions/A/SDL2_ttf")
            message(STATUS "Using bundled SDL frameworks")
        elseif(WIN32
               AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/SDL2.lib"
               AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_img/lib/SDL2_image.lib"
               AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_mix/lib/SDL2_mixer.lib"
               AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf/lib/SDL2_ttf.lib")
            set(SDL2_LINK_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/SDL2.lib")
            set(SDL2_IMAGE_LINK_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_img/lib/SDL2_image.lib")
            set(SDL2_MIXER_LINK_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_mix/lib/SDL2_mixer.lib")
            set(SDL2_TTF_LINK_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf/lib/SDL2_ttf.lib")
            message(STATUS "Using bundled SDL .lib files")
        else()
            message(FATAL_ERROR
                "Bundled SDL fallback was requested but required bundled binaries were not found. "
                "Install system SDL packages or provide bundled libs/frameworks for this platform.")
        endif()
    endif()
endif()

target_link_libraries(${ENGINE_TARGET} PRIVATE
    ${SDL2_LINK_TARGET}
    ${SDL2_IMAGE_LINK_TARGET}
    ${SDL2_MIXER_LINK_TARGET}
    ${SDL2_TTF_LINK_TARGET}
)

if(ANDROID)
    target_compile_definitions(${ENGINE_TARGET} PRIVATE __ANDROID__)
    target_link_libraries(${ENGINE_TARGET} PRIVATE
        android
        log
        GLESv2
    )
endif()
