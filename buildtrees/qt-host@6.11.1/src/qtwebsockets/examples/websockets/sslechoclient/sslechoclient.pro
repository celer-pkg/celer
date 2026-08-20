QT = websockets

TARGET = sslechoclient
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    sslechoclient.cpp

HEADERS += \
    sslechoclient.h

resources.files = ../sslechoserver/localhost.cert
resources.prefix = /

RESOURCES += resources

target.path = $$[QT_INSTALL_EXAMPLES]/websockets/sslechoclient
INSTALLS += target
