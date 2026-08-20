# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_library(ActiveQt INTERFACE)

# ensure that QtCore include path is passed to QDoc on documentation builds
# on non-Windows platforms
if(NOT WIN32)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.19")
    target_link_libraries(ActiveQt INTERFACE Qt::Core)
  endif()
endif()

qt_internal_add_docs(ActiveQt activeqt.qdocconf)
