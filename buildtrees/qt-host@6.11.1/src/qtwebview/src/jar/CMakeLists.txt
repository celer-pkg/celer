# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


qt_internal_add_jar(Qt${QtWebView_VERSION_MAJOR}AndroidWebView
    INCLUDE_JARS ${QT_ANDROID_JAR}
    SOURCES src/org/qtproject/qt/android/view/QtAndroidWebViewController.java
    OUTPUT_DIR "${QT_BUILD_DIR}/jar"
)

qt_path_join(destination ${INSTALL_DATADIR} "jar")

install_jar(Qt${QtWebView_VERSION_MAJOR}AndroidWebView
    DESTINATION ${destination}
    COMPONENT Devel
)
