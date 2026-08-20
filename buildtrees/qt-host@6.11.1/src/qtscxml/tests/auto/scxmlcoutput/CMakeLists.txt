# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (TARGET Qt::Scxml)
    add_subdirectory(default)
    add_subdirectory(namespace)
    add_subdirectory(path)
    add_subdirectory(arguments)
endif()
