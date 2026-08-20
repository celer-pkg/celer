# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


add_subdirectory(repparser)
if(QT_FEATURE_localserver)
    add_subdirectory(remoteobjects)
    if(TARGET Qt::Quick)
        add_subdirectory(remoteobjectsqml)
    endif()
endif()
