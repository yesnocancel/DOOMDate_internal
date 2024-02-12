cmake_minimum_required(VERSION 3.14)
set(CMAKE_C_STANDARD 11)
set(ENVSDK $ENV{PLAYDATE_SDK_PATH})

if (NOT ${ENVSDK} STREQUAL "")
	# Convert path from Windows
	file(TO_CMAKE_PATH ${ENVSDK} SDK)
else()
	execute_process(
			COMMAND bash -c "egrep '^\\s*SDKRoot' $HOME/.Playdate/config"
			COMMAND head -n 1
			COMMAND cut -c9-
			OUTPUT_VARIABLE SDK
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

if (NOT EXISTS ${SDK})
	message(FATAL_ERROR "SDK Path not found; set ENV value PLAYDATE_SDK_PATH")
	return()
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

# Game Name Customization
set(PLAYDATE_GAME_NAME doomdate)
set(PLAYDATE_GAME_DEVICE doomdate_DEVICE)

project(${PLAYDATE_GAME_NAME} C ASM)

set(DOOM_FILES src/am_map.c src/d_items.c src/d_main.c src/d_net.c src/DOOM.c src/doomdef.c src/doomstat.c src/dstrings.c src/f_finale.c src/f_wipe.c src/g_game.c src/hu_lib.c src/hu_stuff.c src/i_net.c src/i_sound.c src/i_system.c src/i_video.c src/info.c src/m_argv.c src/m_bbox.c src/m_cheat.c src/m_fixed.c src/m_menu.c src/m_misc.c src/m_random.c src/m_swap.c src/p_ceilng.c src/p_doors.c src/p_enemy.c src/p_floor.c src/p_inter.c src/p_lights.c src/p_map.c src/p_maputl.c src/p_mobj.c src/p_plats.c src/p_pspr.c src/p_saveg.c src/p_setup.c src/p_sight.c src/p_spec.c src/p_switch.c src/p_telept.c src/p_tick.c src/p_user.c src/r_bsp.c src/r_data.c src/r_draw.c src/r_main.c src/r_plane.c src/r_segs.c src/r_sky.c src/r_things.c src/s_sound.c src/sounds.c src/st_lib.c src/st_stuff.c src/tables.c src/v_video.c src/w_wad.c src/wi_stuff.c src/z_zone.c)
set(PLAYDATE_FILES src/playdate.c src/playdate_sys.c src/bluenoise.h)

if (TOOLCHAIN STREQUAL "armgcc")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsigned-char")
	add_executable(${PLAYDATE_GAME_DEVICE} ${DOOM_FILES} ${PLAYDATE_FILES})
else()
	add_library(${PLAYDATE_GAME_NAME} SHARED ${DOOM_FILES} ${PLAYDATE_FILES})
endif()

include(${SDK}/C_API/buildsupport/playdate_game.cmake)

