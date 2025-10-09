## Build LuaJIT with CMake.
## Copyright (C) 2025 Fatshark Studios AB. All rights reserved.

cmake_minimum_required(VERSION 3.22)
project(luajit-fatshark VERSION 2.1 LANGUAGES NONE)
set(LJ_S ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LJ_B ${CMAKE_CURRENT_BINARY_DIR})

set(SPM_REVISION "r2") # Increment before uploading a new version to S3.
set(SPM_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${SPM_REVISION}")
set(SPM_ZIP "${CMAKE_CURRENT_SOURCE_DIR}/${SPM_NAME}.zip")

# Outputs.
list(APPEND LJ_BINARIES ${LJ_B}/luajit.exe)
list(APPEND LJ_SRC_HEADERS ${LJ_S}/lua.hpp ${LJ_S}/lua.h ${LJ_S}/luaconf.h
  ${LJ_S}/lauxlib.h ${LJ_S}/lualib.h ${LJ_S}/luajit.h
)
set(LJ_LIB_WIN64 ${LJ_B}/lua51_win_x64.lib)
set(LJ_LIB_XBS ${LJ_B}/lua51_xbs.lib)
set(LJ_LIB_PS5 ${LJ_B}/libluajit_ps5.a)
set(LJ_LIB_LINUX ${LJ_B}/libluajit_linux_x64.a)
set(LJ_LIB_LINUX_ARM ${LJ_B}/libluajit_linux_aarch64.a)
list(APPEND LJ_LIBS ${LJ_LIB_WIN64} ${LJ_LIB_PS5} ${LJ_LIB_LINUX}
  #${LJ_LIB_LINUX_ARM} ${LJ_LIB_XBS} # Temporarily disabled.
)
list(APPEND LJ_SRC_ETC ${CMAKE_CURRENT_SOURCE_DIR}/etc/luajit.natvis)

##############################################################################
# SPM & installation.
##############################################################################
install(PROGRAMS ${LJ_BINARIES} DESTINATION bin)
install(FILES ${LJ_SRC_HEADERS} DESTINATION include/lua)
install(FILES ${LJ_LIBS} DESTINATION lib)
install(FILES ${LJ_SRC_ETC} DESTINATION etc)

set(CMAKE_BUILD_PARALLEL_LEVEL 1) # Parallel builds are *not* supported.
set(CMAKE_CONFIGURATION_TYPES Debug;Dev;Release CACHE STRING "" FORCE)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(NOT DEFINED ENV{SR_LIB_DIR})
    message(FATAL_ERROR "SR_LIB_DIR must be set")
  endif()
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "$ENV{SR_LIB_DIR}/${SPM_NAME}")
endif()

add_custom_command(OUTPUT ${SPM_ZIP}
  COMMAND ${CMAKE_COMMAND} -E tar cv ${SPM_ZIP} --format=zip -- ${LJ_LIBS}
  DEPENDS ${LJ_LIBS}
)

# Phony targets.
add_custom_target(win64 DEPENDS ${LJ_LIB_WIN64})
add_custom_target(xbs DEPENDS ${LJ_LIB_XBS})
add_custom_target(ps5 DEPENDS ${LJ_LIB_PS5})
add_custom_target(linux DEPENDS ${LJ_LIB_LINUX})
add_custom_target(linux_arm DEPENDS ${LJ_LIB_LINUX_ARM})
add_custom_target(luajit ALL DEPENDS ${SPM_ZIP})

##############################################################################
# Per-platform buildscript recipes.
##############################################################################
## Glob all sources, minus generated files.
file(GLOB_RECURSE LJ_SOURCES ${LJ_S}/*.[ch] ${LJ_S}/*.lua ${LJ_S}/*.dasc dynasm/*)
list(REMOVE_ITEM LJ_SOURCES ${LJ_S}/host/buildvm_arch.h ${LJ_S}/lj_bcdef.h
  ${LJ_S}/lj_ffdef.h ${LJ_S}/lj_libdef.h ${LJ_S}/lj_recdef.h ${LJ_S}/lj_folddef.h
  ${LJ_S}/luajit.h ${LJ_S}/jit/vmdef.lua 
)
target_sources(luajit PRIVATE ${LJ_SOURCES})
# Windows.
add_custom_command(OUTPUT luajit.exe ${LJ_LIB_WIN64}
  COMMAND ./msvcbuild.bat $<$<CONFIG:debug>:debug> static
  COMMAND ${CMAKE_COMMAND} -E copy lua51.lib ${LJ_LIB_WIN64}
  COMMAND ${CMAKE_COMMAND} -E copy luajit.exe ${LJ_B}/luajit.exe
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)
# XB1.
add_custom_command(OUTPUT ${LJ_LIB_XBS}
  COMMAND ./xb1build.bat $<$<CONFIG:debug>:debug>
  COMMAND ${CMAKE_COMMAND} -E copy luajit.lib ${LJ_LIB_XBS}
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)
# PS5.
add_custom_command(OUTPUT ${LJ_LIB_PS5}
  COMMAND ./ps5build.bat $<$<CONFIG:Debug>:debug>
  COMMAND ${CMAKE_COMMAND} -E copy libluajit$<$<CONFIG:Debug>:D>_ps5.a
                                    ${LJ_LIB_PS5}
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)
# Linux.
add_custom_command(OUTPUT ${LJ_LIB_LINUX}
  COMMAND wsl -e make
  COMMAND ${CMAKE_COMMAND} -E copy libluajit.a ${LJ_LIB_LINUX}
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)
# Linux (ARM64).
add_custom_command(OUTPUT ${LJ_LIB_LINUX_ARM}
  COMMAND wsl -e make CROSS=aarch64-linux-gnu-
  COMMAND ${CMAKE_COMMAND} -E copy libluajit.a ${LJ_LIB_LINUX_ARM}
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)