# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(abstractoauth2)
add_subdirectory(cmake)
add_subdirectory(oauth1)
add_subdirectory(oauth2codeflow)
add_subdirectory(oauth2deviceflow)
add_subdirectory(oauth1signature)
add_subdirectory(oauthhttpserverreplyhandler)
if(QT_FEATURE_urischeme_replyhandler)
    add_subdirectory(oauthurischemereplyhandler)
endif()
if(QT_FEATURE_private_tests)
    add_subdirectory(abstractoauth)
endif()
