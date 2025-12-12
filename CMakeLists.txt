## Buildscript to generate LuaJIT spm package.
## Copyright (C) 2025 Fatshark Studios AB. All rights reserved.

cmake_minimum_required(VERSION 3.22)
project(luajit-fatshark VERSION 2.1 LANGUAGES NONE)

set(CMAKE_CONFIGURATION_TYPES Debug;Dev;Release CACHE STRING "" FORCE)

set(SPM_REVISION "r2") # Increment before uploading a new version.
set(SPM_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${SPM_REVISION}")

##############################################################################
# Config.
##############################################################################
set(LJ_S ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LJ_B ${CMAKE_CURRENT_BINARY_DIR})

# Sources.
file(GLOB_RECURSE LJ_SOURCES ${LJ_S}/*.[ch] ${LJ_S}/*.lua ${LJ_S}/*.dasc dynasm/*)
list(REMOVE_ITEM LJ_SOURCES ${LJ_S}/host/buildvm_arch.h ${LJ_S}/lj_bcdef.h
  ${LJ_S}/lj_ffdef.h ${LJ_S}/lj_libdef.h ${LJ_S}/lj_recdef.h
  ${LJ_S}/lj_folddef.h ${LJ_S}/luajit.h ${LJ_S}/jit/vmdef.lua
)

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
list(APPEND LJ_LIBRARIES ${LJ_LIB_WIN64} ${LJ_LIB_XBS}
  ${LJ_LIB_PS5} ${LJ_LIB_LINUX} ${LJ_LIB_LINUX_ARM}
)
set(LJ_SHARE ${CMAKE_CURRENT_SOURCE_DIR}/etc/luajit32.natvis)
set(LJ_SHARE ${CMAKE_CURRENT_SOURCE_DIR}/etc/luajit64.natvis)
file(GLOB LJ_SHARE_JIT ${LJ_S}/jit/*.lua)

##############################################################################
# Per-platform buildscript recipes.
##############################################################################
# Outputs and intermediate file names are shared between platforms. Thus:
# (1) Recipes must always be rebuilt.
# (2) Outputs must be renamed.
# (3) Parallelism must be disabled.
set(CMAKE_BUILD_PARALLEL_LEVEL 1)

# Windows.
add_custom_target(win64
  BYPRODUCTS luajit.exe ${LJ_LIB_WIN64}
  COMMAND ./msvcbuild.bat $<$<CONFIG:debug>:debug> static
  COMMAND ${CMAKE_COMMAND} -E copy lua51.lib ${LJ_LIB_WIN64}
  COMMAND ${CMAKE_COMMAND} -E copy luajit.exe ${LJ_B}/luajit.exe
  WORKING_DIRECTORY ${LJ_S} SOURCES ${LJ_SOURCES}
)
# XBS.
add_custom_target(xbs
  BYPRODUCTS ${LJ_LIB_XBS}
  COMMAND ./msvcbuild.bat GamingXboxScarlettVS2019 $<$<CONFIG:debug>:debug> static
  COMMAND ${CMAKE_COMMAND} -E copy lua51.lib ${LJ_LIB_XBS}
  WORKING_DIRECTORY ${LJ_S} SOURCES ${LJ_SOURCES}
)
# PS5.
add_custom_target(ps5
  BYPRODUCTS ${LJ_LIB_PS5}
  COMMAND ./ps5build.bat $<$<CONFIG:Debug>:debug>
  COMMAND ${CMAKE_COMMAND} -E copy libluajit$<$<CONFIG:Debug>:D>_ps5.a
                                    ${LJ_LIB_PS5}
  WORKING_DIRECTORY ${LJ_S} SOURCES ${LJ_SOURCES}
)
# Linux.
add_custom_target(linux
  BYPRODUCTS ${LJ_LIB_LINUX}
  COMMAND wsl -e make clean all $<$<CONFIG:Debug>:CCDEBUG=-g>
  COMMAND ${CMAKE_COMMAND} -E copy libluajit.a ${LJ_LIB_LINUX}
  WORKING_DIRECTORY ${LJ_S} SOURCES ${LJ_SOURCES}
)
# Linux arm4.
set(CLANG_ARM64 "clang --target=aarch64-linux-gnu-clang --sysroot=/usr/aarch64-linux-gnu")
add_custom_target(linux_arm
  BYPRODUCTS ${LJ_LIB_ARM}
  COMMAND wsl -e make clean all $<$<CONFIG:Debug>:CCDEBUG=-g>
          CC=clang
          CROSS="aarch64-linux-gnu-"
          STATIC_CC="${CLANG_ARM64}"
          DYNAMIC_CC="${CLANG_ARM64} -fPIC"
          TARGET_LD="${CLANG_ARM64} -fuse-ld=lld"
  COMMAND ${CMAKE_COMMAND} -E copy libluajit.a ${LJ_LIB_LINUX_ARM}
  WORKING_DIRECTORY ${LJ_S} SOURCES ${LJ_SOURCES}
)

##############################################################################
# SPM & installation.
##############################################################################
install(PROGRAMS ${LJ_BINARIES} DESTINATION bin)
install(FILES ${LJ_SRC_HEADERS} DESTINATION include/lua)
install(FILES ${LJ_LIBRARIES} DESTINATION lib)
install(FILES ${LJ_SHARE} DESTINATION share)
install(FILES ${LJ_SHARE_JIT} ${LJ_SHARE_JIT} DESTINATION share/jit)

# Phony target.
add_custom_target(luajit ALL DEPENDS win64 xbs ps5 linux linux_arm)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(NOT DEFINED ENV{SR_LIB_DIR})
    message(FATAL_ERROR "SR_LIB_DIR must be set")
  endif()
  cmake_path(CONVERT $ENV{SR_LIB_DIR} TO_CMAKE_PATH_LIST SR_LIB_DIR)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE ${SR_LIB_DIR}/${SPM_NAME})
endif()

set(CPACK_PACKAGE_FILE_NAME ${SPM_NAME})
include(CPack) # Should be last.