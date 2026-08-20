# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qcgallery LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick ShaderTools CanvasPainter)

qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(qcgalleryexample
    WIN32
    MACOSX_BUNDLE

    main.cpp
    galleryitem.cpp galleryitem.h
    galleryitemrenderer.cpp galleryitemrenderer.h
)

qt_add_qml_module(qcgalleryexample
    URI
        GalleryExample
    QML_FILES
        main.qml
        TopBar.qml
    RESOURCE_PREFIX
        /
    RESOURCES
        images/pattern1.png
        images/pattern2.png
        fonts/Pacifico.ttf
        images/pattern3.png
        images/qt_development_white.png
        images/face-smile-bw.png
    NO_RESOURCE_TARGET_PATH
)

target_link_libraries(qcgalleryexample PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::CanvasPainter
)

qc_add_shaders(qcgalleryexample "qcgallery_custombrush_shaders"
    PREFIX
        "/qcgalleryexample"
    FILES
        brush1.frag
        brush2.frag
        brush3.frag
        brush4.frag
        brush3.vert
)

install(TARGETS qcgalleryexample
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET qcgalleryexample
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
