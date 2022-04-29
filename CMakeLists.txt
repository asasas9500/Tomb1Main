cmake_minimum_required(VERSION 3.11)
project(Tomb1Main C)

set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(FindGit)

set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)

add_library(zlib INTERFACE IMPORTED)
find_library(ZLIB_LIBRARY zlib REQUIRED)
target_link_libraries(zlib INTERFACE "${ZLIB_LIBRARY}")

foreach(NAME libavcodec libavformat libavutil libswresample libswscale)
    string(TOUPPER ${NAME} NAME_UPPERCASE)
    string(SUBSTRING ${NAME} 3 -1 NAME_NAKED)
    add_library(${NAME} INTERFACE IMPORTED)
    find_library(${NAME_UPPERCASE}_LIBRARY ${NAME} REQUIRED)
    target_link_libraries(${NAME} INTERFACE "${${NAME_UPPERCASE}_LIBRARY}")
endforeach()

add_library(sdl2 INTERFACE IMPORTED)
find_library(SDL2_LIBRARY SDL2 REQUIRED)
find_library(SDL2MAIN_LIBRARY SDL2main REQUIRED)
target_link_libraries(sdl2 INTERFACE "${SDL2_LIBRARY}" "${SDL2MAIN_LIBRARY}")

add_executable(
    Tomb1Main
    "${CMAKE_SOURCE_DIR}/src/3dsystem/3d_gen.c"
    "${CMAKE_SOURCE_DIR}/src/3dsystem/matrix.c"
    "${CMAKE_SOURCE_DIR}/src/3dsystem/phd_math.c"
    "${CMAKE_SOURCE_DIR}/src/config.c"
    "${CMAKE_SOURCE_DIR}/src/filesystem.c"
    "${CMAKE_SOURCE_DIR}/src/game/box.c"
    "${CMAKE_SOURCE_DIR}/src/game/camera.c"
    "${CMAKE_SOURCE_DIR}/src/game/cinema.c"
    "${CMAKE_SOURCE_DIR}/src/game/clock.c"
    "${CMAKE_SOURCE_DIR}/src/game/collide.c"
    "${CMAKE_SOURCE_DIR}/src/game/control.c"
    "${CMAKE_SOURCE_DIR}/src/game/control_pause.c"
    "${CMAKE_SOURCE_DIR}/src/game/control_util.c"
    "${CMAKE_SOURCE_DIR}/src/game/demo.c"
    "${CMAKE_SOURCE_DIR}/src/game/draw.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/bubbles.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/chain_block.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/dino_stomp.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/earthquake.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/explosion.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/finish_level.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/flicker.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/flipmap.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/flood.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/lara_effects.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/powerup.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/raising_block.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/sand.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/stairs2slope.c"
    "${CMAKE_SOURCE_DIR}/src/game/effect_routines/turn_180.c"
    "${CMAKE_SOURCE_DIR}/src/game/effects/blood.c"
    "${CMAKE_SOURCE_DIR}/src/game/effects/exploding_death.c"
    "${CMAKE_SOURCE_DIR}/src/game/effects/gun.c"
    "${CMAKE_SOURCE_DIR}/src/game/effects/gunshot.c"
    "${CMAKE_SOURCE_DIR}/src/game/fmv.c"
    "${CMAKE_SOURCE_DIR}/src/game/game.c"
    "${CMAKE_SOURCE_DIR}/src/game/gamebuf.c"
    "${CMAKE_SOURCE_DIR}/src/game/gameflow.c"
    "${CMAKE_SOURCE_DIR}/src/game/gun/gun.c"
    "${CMAKE_SOURCE_DIR}/src/game/gun/gun_misc.c"
    "${CMAKE_SOURCE_DIR}/src/game/gun/gun_pistols.c"
    "${CMAKE_SOURCE_DIR}/src/game/gun/gun_rifle.c"
    "${CMAKE_SOURCE_DIR}/src/game/hair.c"
    "${CMAKE_SOURCE_DIR}/src/game/input.c"
    "${CMAKE_SOURCE_DIR}/src/game/inventry.c"
    "${CMAKE_SOURCE_DIR}/src/game/invfunc.c"
    "${CMAKE_SOURCE_DIR}/src/game/invvars.c"
    "${CMAKE_SOURCE_DIR}/src/game/items.c"
    "${CMAKE_SOURCE_DIR}/src/game/lara/lara.c"
    "${CMAKE_SOURCE_DIR}/src/game/lara/lara_col.c"
    "${CMAKE_SOURCE_DIR}/src/game/lara/lara_control.c"
    "${CMAKE_SOURCE_DIR}/src/game/lara/lara_draw.c"
    "${CMAKE_SOURCE_DIR}/src/game/lara/lara_look.c"
    "${CMAKE_SOURCE_DIR}/src/game/lara/lara_misc.c"
    "${CMAKE_SOURCE_DIR}/src/game/lara/lara_state.c"
    "${CMAKE_SOURCE_DIR}/src/game/level.c"
    "${CMAKE_SOURCE_DIR}/src/game/lot.c"
    "${CMAKE_SOURCE_DIR}/src/game/music.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/ape.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/bacon_lara.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/baldy.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/bat.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/bear.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/centaur.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/cowboy.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/crocodile.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/larson.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/lion.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/mummy.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/mutant.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/natla.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/pierre.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/pod.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/raptor.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/rat.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/skate_kid.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/statue.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/torso.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/trex.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/ai/wolf.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/boat.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/bridge.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/cabin.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/cog.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/door.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/blood.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/body_part.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/bubble.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/earthquake.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/explosion.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/gunshot.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/missile.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/ricochet.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/splash.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/twinkle.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/effects/waterfall.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/keyhole.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/misc.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/pickup.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/puzzle_hole.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/save_crystal.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/scion.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/switch.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/trapdoor.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/damocles_sword.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/dart.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/falling_block.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/falling_ceiling.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/flame.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/lava.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/lightning_emitter.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/midas_touch.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/movable_block.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/pendulum.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/rolling_ball.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/rolling_block.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/spikes.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/teeth_trap.c"
    "${CMAKE_SOURCE_DIR}/src/game/objects/traps/thors_hammer.c"
    "${CMAKE_SOURCE_DIR}/src/game/option.c"
    "${CMAKE_SOURCE_DIR}/src/game/option_compass.c"
    "${CMAKE_SOURCE_DIR}/src/game/option_control.c"
    "${CMAKE_SOURCE_DIR}/src/game/option_graphics.c"
    "${CMAKE_SOURCE_DIR}/src/game/option_passport.c"
    "${CMAKE_SOURCE_DIR}/src/game/option_sound.c"
    "${CMAKE_SOURCE_DIR}/src/game/output.c"
    "${CMAKE_SOURCE_DIR}/src/game/overlay.c"
    "${CMAKE_SOURCE_DIR}/src/game/people.c"
    "${CMAKE_SOURCE_DIR}/src/game/picture.c"
    "${CMAKE_SOURCE_DIR}/src/game/random.c"
    "${CMAKE_SOURCE_DIR}/src/game/requester.c"
    "${CMAKE_SOURCE_DIR}/src/game/room.c"
    "${CMAKE_SOURCE_DIR}/src/game/savegame.c"
    "${CMAKE_SOURCE_DIR}/src/game/savegame_bson.c"
    "${CMAKE_SOURCE_DIR}/src/game/savegame_legacy.c"
    "${CMAKE_SOURCE_DIR}/src/game/screen.c"
    "${CMAKE_SOURCE_DIR}/src/game/settings.c"
    "${CMAKE_SOURCE_DIR}/src/game/setup.c"
    "${CMAKE_SOURCE_DIR}/src/game/shell.c"
    "${CMAKE_SOURCE_DIR}/src/game/sound.c"
    "${CMAKE_SOURCE_DIR}/src/game/sphere.c"
    "${CMAKE_SOURCE_DIR}/src/game/stats.c"
    "${CMAKE_SOURCE_DIR}/src/game/text.c"
    "${CMAKE_SOURCE_DIR}/src/game/viewport.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/2d/2d_renderer.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/2d/2d_surface.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/3d/3d_renderer.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/3d/vertex_stream.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/blitter.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/context.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/gl/buffer.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/gl/gl_core_3_3.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/gl/program.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/gl/sampler.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/gl/texture.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/gl/utils.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/gl/vertex_array.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/gl/wgl_ext.c"
    "${CMAKE_SOURCE_DIR}/src/gfx/screenshot.c"
    "${CMAKE_SOURCE_DIR}/src/global/vars.c"
    "${CMAKE_SOURCE_DIR}/src/global/vars_platform.c"
    "${CMAKE_SOURCE_DIR}/src/init.c"
    "${CMAKE_SOURCE_DIR}/src/json/bson_parse.c"
    "${CMAKE_SOURCE_DIR}/src/json/bson_write.c"
    "${CMAKE_SOURCE_DIR}/src/json/json_base.c"
    "${CMAKE_SOURCE_DIR}/src/json/json_parse.c"
    "${CMAKE_SOURCE_DIR}/src/json/json_write.c"
    "${CMAKE_SOURCE_DIR}/src/log.c"
    "${CMAKE_SOURCE_DIR}/src/memory.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_audio.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_audio_sample.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_audio_stream.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_clock.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_filesystem.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_fmv.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_input.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_misc.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_output.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_picture.c"
    "${CMAKE_SOURCE_DIR}/src/specific/s_shell.c"
)

if(GIT_FOUND)
    file(TO_NATIVE_PATH "${GIT_EXECUTABLE}" NATIVE_GIT)
    file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}" NATIVE_SOURCE_DIR)

    execute_process(
        COMMAND "${NATIVE_GIT}" describe --abbrev=7 --tags
        WORKING_DIRECTORY "${NATIVE_SOURCE_DIR}"
        OUTPUT_VARIABLE VERSION_PRETTY
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

target_compile_definitions(Tomb1Main PRIVATE VERSION="T1M${VERSION_PRETTY}")
target_compile_options(Tomb1Main PRIVATE /W3 /wd4547 /wd4548 /wd4549 /wd4552 /wd4553 /wd4555 /MT)
target_include_directories(Tomb1Main PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src" "${CMAKE_PREFIX_PATH}/include")
target_link_libraries(Tomb1Main PRIVATE bcrypt imagehlp libavcodec libavformat libavutil dinput8 dxguid gdi32 imm32 opengl32 libswresample libswscale mfplat mfuuid ole32 oleaut32 setupapi sdl2 shell32 strmiids user32 uuid version winmm zlib)
