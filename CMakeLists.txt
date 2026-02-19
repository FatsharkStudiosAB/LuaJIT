## Buildscript to generate LuaJIT spm package.
## Copyright (C) 2025 Fatshark Studios AB. All rights reserved.

cmake_minimum_required(VERSION 3.22)
project(luajit VERSION 2.1 LANGUAGES NONE)
set(CMAKE_GENERATOR_PLATFORM x64)

set(CMAKE_CONFIGURATION_TYPES Debug;Release CACHE STRING "" FORCE)

set(SPM_REVISION "r1")
set(SPM_VERSION "${CMAKE_PROJECT_VERSION}.fatshark_v5")
set(SPM_NAME "${CMAKE_PROJECT_NAME}-${SPM_VERSION}-${SPM_REVISION}")

##############################################################################
# Config.
##############################################################################
set(LJ_S ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LJ_B ${CMAKE_CURRENT_BINARY_DIR})

# Sources. Reconfiguration is needed when adding or removing files.
file(GLOB_RECURSE LJ_SOURCES ${LJ_S}/*.[ch] ${LJ_S}/*.lua ${LJ_S}/*.dasc dynasm/*)
list(REMOVE_ITEM LJ_SOURCES ${LJ_S}/host/buildvm_arch.h ${LJ_S}/lj_bcdef.h
  ${LJ_S}/lj_ffdef.h ${LJ_S}/lj_libdef.h ${LJ_S}/lj_recdef.h
  ${LJ_S}/lj_folddef.h ${LJ_S}/luajit.h ${LJ_S}/jit/vmdef.lua
)

# Cache variables.
set(USE_GC32 ON CACHE STRING "Use 32-bit GC references.")
set(WSL_DISTRO "Ubuntu-24.04" CACHE STRING "WSL distribution to build Linux.")
set(CLANG_ARM64_FLAGS "--target=aarch64-linux-gnu" CACHE STRING "ARM64 cross-compilation flags.")

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
if(USE_GC32)
  set(LJ_SHARE_NATVIS ${CMAKE_CURRENT_SOURCE_DIR}/etc/luajit32.natvis)
else()
  set(LJ_SHARE_NATVIS ${CMAKE_CURRENT_SOURCE_DIR}/etc/luajit64.natvis)
endif()
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
add_custom_command(OUTPUT ${LJ_LIB_WIN64}
  BYPRODUCTS ${LJ_B}/luajit.exe
  COMMAND ./msvcbuild.bat $<$<BOOL:${USE_GC32}>:nogc64> $<$<CONFIG:DEBUG>:debug> static
  COMMAND ${CMAKE_COMMAND} -E copy lua51.lib ${LJ_LIB_WIN64}
  COMMAND ${CMAKE_COMMAND} -E copy luajit.exe ${LJ_B}/luajit.exe
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES} ${LJ_SHARE_NATVIS}
)
add_custom_target(win64 ALL DEPENDS ${LJ_LIB_WIN64} SOURCES ${LJ_SOURCES})
# XBS.
add_custom_command(OUTPUT ${LJ_LIB_XBS}
  COMMAND ./msvcbuild.bat GamingXboxScarlettVS2019 $<$<BOOL:${USE_GC32}>:nogc64> $<$<CONFIG:DEBUG>:debug> static
  COMMAND ${CMAKE_COMMAND} -E copy lua51.lib ${LJ_LIB_XBS}
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)
add_custom_target(xbs ALL DEPENDS ${LJ_LIB_XBS} SOURCES ${LJ_SOURCES})
# PS5.
add_custom_command(OUTPUT ${LJ_LIB_PS5}
  COMMAND ./ps5build.bat $<$<BOOL:${USE_GC32}>:gc32> $<$<CONFIG:DEBUG>:debug>
  COMMAND ${CMAKE_COMMAND} -E copy libluajit$<$<CONFIG:DEBUG>:D>_ps5.a
                                    ${LJ_LIB_PS5}
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)
add_custom_target(ps5 ALL DEPENDS ${LJ_LIB_PS5} SOURCES ${LJ_SOURCES}
)
# Linux.
add_custom_command(OUTPUT ${LJ_LIB_LINUX}
  COMMAND wsl -d ${WSL_DISTRO} -e make clean all $<$<CONFIG:DEBUG>:CCDEBUG=-g> $<$<BOOL:${USE_GC32}>:XCFLAGS=-DLUAJIT_DISABLE_GC64>
          BUILDMODE=static
          CC="clang -fPIC"
  COMMAND ${CMAKE_COMMAND} -E copy libluajit.a ${LJ_LIB_LINUX}
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)
add_custom_target(linux ALL DEPENDS ${LJ_LIB_LINUX} SOURCES ${LJ_SOURCES})
# Linux aarch64.
add_custom_command(
  OUTPUT ${LJ_LIB_LINUX_ARM}
  COMMAND wsl -d ${WSL_DISTRO} -e make clean all $<$<CONFIG:DEBUG>:CCDEBUG=-g> $<$<BOOL:${USE_GC32}>:XCFLAGS=-DLUAJIT_DISABLE_GC64>
          BUILDMODE=static
          CC=clang
          CROSS="aarch64-linux-gnu-"
          STATIC_CC="clang ${CLANG_ARM64_FLAGS} -fPIC"
          TARGET_LD="clang ${CLANG_ARM64_FLAGS} -fuse-ld=lld"
  COMMAND ${CMAKE_COMMAND} -E copy libluajit.a ${LJ_LIB_LINUX_ARM}
  WORKING_DIRECTORY ${LJ_S} DEPENDS ${LJ_SOURCES}
)
add_custom_target(linux_arm ALL DEPENDS ${LJ_LIB_LINUX_ARM} SOURCES ${LJ_SOURCES})

##############################################################################
# Visual Studio
##############################################################################
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT win64)
set_target_properties(win64 PROPERTIES
  VS_DEBUGGER_COMMAND "${LJ_B}/luajit.exe"
  VS_DEBUGGER_WORKING_DIRECTORY "${LJ_S}"
)

##############################################################################
# SPM & installation.
##############################################################################
install(PROGRAMS ${LJ_BINARIES} DESTINATION "bin")
install(FILES ${LJ_SRC_HEADERS} DESTINATION "include/lua")
install(FILES ${LJ_LIBRARIES} DESTINATION "lib")
install(FILES ${LJ_SHARE_NATVIS} DESTINATION "share" RENAME "luajit.natvis")
install(FILES ${LJ_SHARE_JIT} DESTINATION "share/jit")

# Install prefix always set regardless of CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT.
if(NOT DEFINED ENV{SR_LIB_DIR})
  message(FATAL_ERROR "Either the install prefix or SR_LIB_DIR must be set.")
endif()
cmake_path(CONVERT $ENV{SR_LIB_DIR} TO_CMAKE_PATH_LIST SR_LIB_DIR)
set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "${SR_LIB_DIR}/${SPM_NAME}")

set(CPACK_PACKAGE_FILE_NAME ${SPM_NAME})
include(CPack) # Should be last.