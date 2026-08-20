QT += remoteobjects bluetooth

CONFIG -= app_bundle

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/bleiodevice.h

SOURCES += \
    $$PWD/bleiodevice.cpp

ios: QMAKE_INFO_PLIST = $$PWD/Info.ios.plist
macos: QMAKE_INFO_PLIST = $$PWD/Info.qmake.macos.plist
