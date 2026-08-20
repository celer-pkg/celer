!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

DEFINES += QMAKE_BUILD

SOURCES += main.cpp \
    customformatter.cpp

HEADERS += customformatter.h

RESOURCES += qmlaxishandling.qrc

OTHER_FILES += doc/src/* \
               doc/images/* \
               qml/qmlaxishandling/*
