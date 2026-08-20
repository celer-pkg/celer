# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(SameGame LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Sql)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(samegame
    main.cpp
)

set_target_properties(samegame PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(samegame PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Sql
)

set_source_files_properties(content/Settings.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(samegame
    URI SameGameModule
    QML_FILES
        content/Block.qml
        content/BlockEmitter.qml
        content/Button.qml
        content/GameArea.qml
        content/LogoAnimation.qml
        content/MenuEmitter.qml
        content/PaintEmitter.qml
        content/PrimaryPack.qml
        content/PuzzleBlock.qml
        content/SamegameText.qml
        content/Settings.qml
        content/SimpleBlock.qml
        content/SmokeText.qml
        Main.qml
        content/samegame.js
    RESOURCES
        content/gfx/background-puzzle.png
        content/gfx/background.png
        content/gfx/bar.png
        content/gfx/blue-puzzle.png
        content/gfx/blue.png
        content/gfx/bubble-highscore.png
        content/gfx/bubble-puzzle.png
        content/gfx/but-game-1.png
        content/gfx/but-game-2.png
        content/gfx/but-game-3.png
        content/gfx/but-game-4.png
        content/gfx/but-game-new.png
        content/gfx/but-menu.png
        content/gfx/but-puzzle-next.png
        content/gfx/but-quit.png
        content/gfx/green-puzzle.png
        content/gfx/green.png
        content/gfx/icon-fail.png
        content/gfx/icon-ok.png
        content/gfx/icon-time.png
        content/gfx/logo-a.png
        content/gfx/logo-e.png
        content/gfx/logo-g.png
        content/gfx/logo-m.png
        content/gfx/logo-s.png
        content/gfx/logo.png
        content/gfx/particle-brick.png
        content/gfx/particle-paint.png
        content/gfx/particle-smoke.png
        content/gfx/red-puzzle.png
        content/gfx/red.png
        content/gfx/text-highscore-new.png
        content/gfx/text-highscore.png
        content/gfx/text-no-winner.png
        content/gfx/text-p1-go.png
        content/gfx/text-p1-won.png
        content/gfx/text-p1.png
        content/gfx/text-p2-go.png
        content/gfx/text-p2-won.png
        content/gfx/text-p2.png
        content/gfx/yellow-puzzle.png
        content/gfx/yellow.png
        content/levels/TemplateBase.qml
        content/levels/level0.qml
        content/levels/level1.qml
        content/levels/level2.qml
        content/levels/level3.qml
        content/levels/level4.qml
        content/levels/level5.qml
        content/levels/level6.qml
        content/levels/level7.qml
        content/levels/level8.qml
        content/levels/level9.qml
)

install(TARGETS samegame
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET samegame
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
