TARGET = btscanner

QT = core bluetooth widgets
requires(qtConfig(listwidget))
TEMPLATE = app

SOURCES = \
    main.cpp \
    device.cpp \
    service.cpp

ios: QMAKE_INFO_PLIST = Info.plist
macos: QMAKE_INFO_PLIST = ../../../../examples/bluetooth/shared/Info.qmake.macos.plist

HEADERS = \
    device.h \
    service.h

FORMS = \
    device.ui \
    service.ui

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/btscanner
INSTALLS += target
