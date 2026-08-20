# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#
# abstractviewer library
#

qt_add_library(abstractviewer
    abstractviewer.cpp abstractviewer.h
    translator.cpp translator.h
    abstractviewerglobal.h
)

target_compile_definitions(abstractviewer PRIVATE BUILD_ABSTRACTVIEWER_LIB)

target_include_directories(abstractviewer INTERFACE .)

target_link_libraries(abstractviewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(TARGET Qt6::PrintSupport)
    target_link_libraries(abstractviewer PUBLIC Qt6::PrintSupport)
    target_compile_definitions(abstractviewer PUBLIC
        DOCUMENTVIEWER_PRINTSUPPORT
    )
endif()

#
# documentviewer library
#

set(CMAKE_AUTORCC ON)

qt_add_executable(documentviewer
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    viewerfactory.cpp viewerfactory.h
    recentfiles.cpp recentfiles.h
    recentfilemenu.cpp recentfilemenu.h
    viewerinterfaces.h
    documentviewer.qrc
)

qt_add_translations(documentviewer
    SOURCE_TARGETS documentviewer abstractviewer
    TS_FILE_BASE docviewer
    MERGE_QT_TRANSLATIONS
    QT_TRANSLATION_CATALOGS qtbase
)

set_target_properties(documentviewer PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(documentviewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    abstractviewer
)

if (APPLE)
    set_target_properties(documentviewer PROPERTIES
        OUTPUT_NAME "Document Viewer"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Document Viewer"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (C) The Qt Company Ltd. and other contributors."
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.examples.documentviewer"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

set(plugin_targets
    imageviewer
    jsonviewer
    txtviewer
)

if(TARGET pdfviewer)
    list(APPEND plugin_targets pdfviewer)
endif()

if(TARGET Q3Dviewer)
    list(APPEND plugin_targets Q3Dviewer)
endif()

if(QT6_IS_SHARED_LIBS_BUILD)
    add_dependencies(documentviewer ${plugin_targets})
else()
    target_link_libraries(documentviewer PRIVATE ${plugin_targets})
endif()

#
# installation
#

if(APPLE)
    set(INSTALL_LIBDIR "Document Viewer.app/Contents/Frameworks")
    set(INSTALL_PLUGINSDIR "Document Viewer.app/Contents/PlugIns")
elseif(WIN32)
    set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
    set(INSTALL_LIBDIR ${CMAKE_INSTALL_BINDIR})
    set(INSTALL_PLUGINSDIR "plugins")
else()
    set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
    set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
    set(INSTALL_PLUGINSDIR "plugins")
endif()

install(TARGETS documentviewer abstractviewer
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${INSTALL_BINDIR}"
    LIBRARY DESTINATION "${INSTALL_LIBDIR}"
)

install(TARGETS ${plugin_targets}
    DESTINATION "${INSTALL_PLUGINSDIR}"
)

# Install Qt libraries and plugins
qt_generate_deploy_app_script(
    TARGET documentviewer
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    NO_TRANSLATIONS
)

install(SCRIPT ${deploy_script})
