QT += qml qml-private gui-private quick3d-private quick3druntimerender quick3druntimerender-private

SOURCES += \
        main.cpp \
        parser.cpp \
        genshaders.cpp

HEADERS += \
    parser.h \
    genshaders.h

load(qt_tool)
