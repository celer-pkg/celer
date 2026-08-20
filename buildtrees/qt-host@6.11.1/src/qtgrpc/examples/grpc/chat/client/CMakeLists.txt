# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.18)
project(qtgrpc_chat_client LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()
set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/grpc/chat")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GRPC_CHAT_USE_EMOJI_FONT "Enable emoji font support" OFF)

find_package(Qt6 REQUIRED
    COMPONENTS
        Core
        Gui
        Qml
        Quick
        QuickControls2
        Protobuf
        ProtobufQtCoreTypes
        ProtobufQuick
        Grpc
        GrpcQuick
)
qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(qtgrpc_chat_client WIN32 MACOSX_BUNDLE main.cpp)
qt_add_qml_module(qtgrpc_chat_client
    URI QtGrpcChat
    VERSION 1.0
    QML_FILES
        Main.qml
        ChatView.qml
        LoginView.qml
        LogDialog.qml
        StatusDisplay.qml
        ChatMessages/FileContinuationDelegate.qml
        ChatMessages/FileDelegate.qml
        ChatMessages/ImageDelegate.qml
        ChatMessages/TextDelegate.qml
        ChatMessages/UserStatusDelegate.qml
    RESOURCES
        res/send.svg
        res/generic_file.svg
        res/attach.svg
        res/gear.svg
    SOURCES
        chatengine.h chatengine.cpp
        clientworker.h clientworker.cpp
        userstatusmodel.h userstatusmodel.cpp
        chatmessagemodel.h chatmessagemodel.cpp
        logginginterceptor.h logginginterceptor.cpp
        logmodel.h logmodel.cpp
)

qt_add_resources(qtgrpc_chat_client "resources"
    FILES
        res/root.crt
        res/qtchat_logo.png
)

if(GRPC_CHAT_USE_EMOJI_FONT)
    set(font_path "${CMAKE_CURRENT_BINARY_DIR}/downloads/NotoColorEmoji.ttf")

    if(NOT EXISTS "${font_path}")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/downloads")
        set(font_url "https://github.com/googlefonts/noto-emoji/raw/main/fonts/NotoColorEmoji.ttf")
        file(DOWNLOAD ${font_url} ${font_path} SHOW_PROGRESS)
    endif()

    set_source_files_properties(${font_path} PROPERTIES QT_RESOURCE_ALIAS "NotoColorEmoji.ttf")
    qt_add_resources(qtgrpc_chat_client "font"
        FILES
            "${font_path}"
    )
    target_compile_definitions(qtgrpc_chat_client PRIVATE USE_EMOJI_FONT)
endif()

#! [client-setup-1]
add_library(qtgrpc_chat_client_proto STATIC)
qt_add_protobuf(qtgrpc_chat_client_proto
    QML
    QML_URI QtGrpcChat.Proto
    PROTO_FILES
        ../proto/chatmessages.proto
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufQtCoreTypes,QT_PROTO_INCLUDES>
)

qt_add_grpc(qtgrpc_chat_client_proto CLIENT
    PROTO_FILES
        ../proto/qtgrpcchat.proto
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufQtCoreTypes,QT_PROTO_INCLUDES>
)
#! [client-setup-1]

if(ANDROID)
    set_target_properties(qtgrpc_chat_client
        PROPERTIES
            QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/android"
            QT_ANDROID_PACKAGE_NAME "org.qtproject.grpc.chat"
            QT_ANDROID_APP_NAME "QtGrpcChat"
            QT_ANDROID_APP_ICON "@drawable/icon"
    )

    include(FetchContent)
    FetchContent_Declare(
        android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(android_openssl)
    include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
    add_android_openssl_libraries(qtgrpc_chat_client)
endif()

#! [client-setup-2]
target_link_libraries(qtgrpc_chat_client_proto
    PUBLIC
        Qt6::Protobuf
        Qt6::ProtobufQtCoreTypes
        Qt6::Grpc
)
#! [client-setup-2]

target_link_libraries(qtgrpc_chat_client
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        qtgrpc_chat_client_proto
)

install(TARGETS qtgrpc_chat_client
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

qt_generate_deploy_qml_app_script(
    TARGET qtgrpc_chat_client
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})

if(TARGET qtgrpc_chat_server)
    add_dependencies(qtgrpc_chat_client qtgrpc_chat_server)
endif()
