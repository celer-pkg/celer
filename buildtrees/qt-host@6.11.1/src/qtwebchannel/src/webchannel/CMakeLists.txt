# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT6_INSTALL_QT_SHAREDIR)
  set(INSTALL_WEBCHANNEL_SHAREDIR "${QT6_INSTALL_QT_SHAREDIR}/webchannel")
else()
  set(INSTALL_WEBCHANNEL_SHAREDIR "${INSTALL_QT_SHAREDIR}/webchannel")
endif()

include(CMakePackageConfigHelpers)

set(target "WebChannel")
set(path_suffix "${INSTALL_CMAKE_NAMESPACE}${target}")
qt_path_join(config_build_dir ${QT_CONFIG_BUILD_DIR} ${path_suffix})
qt_path_join(config_install_dir ${QT_CONFIG_INSTALL_DIR} ${path_suffix})

configure_package_config_file(
    Qt6WebChannelExtras.cmake.in
    "${config_build_dir}/Qt6WebChannelExtras.cmake"
    INSTALL_DESTINATION "${config_install_dir}"
    PATH_VARS INSTALL_WEBCHANNEL_SHAREDIR
)

qt_internal_add_module(WebChannel
    SOURCES
        qmetaobjectpublisher.cpp qmetaobjectpublisher_p.h
        qwebchannel.cpp qwebchannel.h qwebchannel_p.h
        qwebchannelabstracttransport.cpp qwebchannelabstracttransport.h
        signalhandler_p.h
        qwebchannelglobal.h
        qwebchannel.js
    DEFINES
        QT_NO_CONTEXTLESS_CONNECT
    LIBRARIES
        Qt::CorePrivate
    EXTRA_CMAKE_FILES
        "${config_build_dir}/Qt6WebChannelExtras.cmake"
    EXTRA_CMAKE_INCLUDES
        Qt6WebChannelExtras.cmake
)

qt_internal_add_resource(WebChannel "resources"
    PREFIX
        "/qtwebchannel/"
    FILES
        qwebchannel.js
)

if(TARGET Qt::Qml)
    qt_internal_extend_target(WebChannel PUBLIC_LIBRARIES Qt::Qml)
else()
    qt_internal_extend_target(WebChannel DEFINES QT_NO_JSVALUE)
endif()

qt_internal_add_docs(WebChannel
    doc/qtwebchannel.qdocconf
)

install(FILES qwebchannel.js
        DESTINATION "${INSTALL_WEBCHANNEL_SHAREDIR}")
file(COPY qwebchannel.js
     DESTINATION "${QT_BUILD_DIR}/${INSTALL_WEBCHANNEL_SHAREDIR}/")
