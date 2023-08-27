# 
# This file is part of the libWetHair open source project
# 
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# Copyright 2022 Yun (Raymond) Fei, Henrique Teles Maia, Christopher Batty,
# Changxi Zheng, and Eitan Grinspun
# 

if(TARGET anttweakbar::anttweakbar)
    return()
elseif(LIBWETHAIR_FIND_DEPENDENCIES)
    find_package(AntTweakBar REQUIRED CONFIG)
    # The AntTweakBar config does not namespace their target
    if(NOT TARGET AntTweakBar::AntTweakBar)
        add_library(AntTweakBar::AntTweakBar ALIAS AntTweakBar)
    endif()
    return()
endif()

message(STATUS "Third-party (external): creating target 'anttweakbar::anttweakbar'")

include(FetchContent)
FetchContent_Declare(
    anttweakbar
    GIT_REPOSITORY https://github.com/nepluno/AntTweakBar.git
    GIT_TAG        7ee2557bdf41fb5c17ad02ee0dab6c52b8cc4e1b
)

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "anttweakbar")
set(ATB_BUILD_EXAMPLES OFF CACHE BOOL "")
FetchContent_MakeAvailable(anttweakbar)

add_library(AntTweakBar::AntTweakBar ALIAS AntTweakBar)

set_target_properties(AntTweakBar PROPERTIES FOLDER third_party)
