#Copyright (C) 2023 The Qt Company Ltd.
#SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
QT += qml quick

CONFIG += c++11

SOURCES += main.cpp

RESOURCES += \
    coffeemachine.qrc

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060000 # disables all APIs deprecated in Qt 6.0.0 and earlier

qnx: target.path = /tmp/$${TARGET}/bin
else: win32|unix: target.path = $$[QT_INSTALL_EXAMPLES]/demos/$${TARGET}
!isEmpty(target.path): INSTALLS += target
