cmake_minimum_required(VERSION 3.16)

if(CMAKE_CROSSCOMPILING AND NOT ANDROID)
    # Cross-compiling to embedded does not pass CI. Re-enable when fixed.
    message(WARNING "Skipping ssgilightmap example because cross-compiling is detected")
    return()
endif()

if(CMAKE_GENERATOR STREQUAL "Xcode" AND IOS)
    # https://bugreports.qt.io/browse/QTBUG-103497
    message(WARNING "Skipping ssgilightmap example because building for iOS")
    return()
endif()

if(EMSCRIPTEN)
    # Emscripten runs out of memory: QTBUG-142678
    message(WARNING "Skipping ssgilightmap example because webassembly detected")
    return()
endif()

include(FetchContent)

project(ssgilightmap VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC OFF)

find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D)

# Archive created from source dir:
# grep -oP '(?<=<file>).*?(?=</file>)' model_assets.qrc | zip -0 model_assets.zip model_assets.qrc -@ && md5sum model_assets.zip
FetchContent_Declare(
  model_assets
  URL https://ordp.qt.io/qt/bundles/SSGI-Lightmap-Demo/SSGI-Lightmap-Demo-model-assets.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  URL_HASH MD5=711d611303a32f2955f81a1d29fb44c8
)

FetchContent_MakeAvailable(model_assets)

qt_standard_project_setup(REQUIRES 6.10)

set(SOURCES main.cpp)
qt_add_big_resources(SOURCES ${model_assets_SOURCE_DIR}/model_assets.qrc OPTIONS -no-compress)
qt_add_big_resources(SOURCES assets.qrc OPTIONS -no-compress)
list(APPEND SOURCES ${model_assets_SOURCE_DIR}/model_assets.qrc)
list(APPEND SOURCES assets.qrc)
set_property(SOURCE ${model_assets_SOURCE_DIR}/model_assets.qrc PROPERTY SKIP_AUTORCC ON)
set_property(SOURCE assets.qrc PROPERTY SKIP_AUTORCC ON)

qt_add_executable(ssgilightmap
    ${SOURCES}
)

qt_add_qml_module(ssgilightmap
    URI app
    VERSION 1.0
    QML_FILES
        Main.qml
        IconButton.qml
        assets/SceneBase.qml
        assets/Bedroom/Bedroom.qml
        assets/CornellBox/CornellBox.qml
        assets/Sponza/Sponza.qml
    RESOURCE_PREFIX
        "/"
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(ssgilightmap PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.app
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(ssgilightmap
    PRIVATE Qt6::Quick Qt6::Quick3D
)

include(GNUInstallDirs)
install(TARGETS ssgilightmap
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

