# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_BUILD_STANDALONE_TESTS)
    # Add qt_find_package calls for extra dependencies that need to be found when building
    # the standalone tests here.
endif()

# androidtestrunner doesn't grant the permissions to tests when installing aab
# using bundletool. Disable QT_USE_ANDROID_MODERN_BUNDLE flag, for
# qtconnectivity tests. See QTBUG-138597.
set(QT_USE_ANDROID_MODERN_BUNDLE OFF)

if(TARGET Qt::Bluetooth)
    if(TARGET Qt::Quick)
        add_subdirectory(bttestui)
    endif()

    add_subdirectory(bluetoothtestdevice)
endif()

qt_build_tests()
