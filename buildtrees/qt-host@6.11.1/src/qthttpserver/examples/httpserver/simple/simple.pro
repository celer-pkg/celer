requires(qtHaveModule(httpserver))

TEMPLATE = app

CONFIG += cmdline
QT = httpserver
android: QT += gui

SOURCES += \
    main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/httpserver/simple
INSTALLS += target

RESOURCES += \
    assets/certificate.crt \
    assets/private.key \
    assets/qt-logo.png

CONFIG += cmdline
