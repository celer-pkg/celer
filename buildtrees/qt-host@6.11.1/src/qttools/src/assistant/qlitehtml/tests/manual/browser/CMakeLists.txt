# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(${Qt} COMPONENTS Widgets Network REQUIRED)

add_executable(qlitehtmlbrowser
  MACOSX_BUNDLE
  main.cpp)

target_link_libraries(qlitehtmlbrowser qlitehtml ${Qt}::Widgets ${Qt}::Network)
