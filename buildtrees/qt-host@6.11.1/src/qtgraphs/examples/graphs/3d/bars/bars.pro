!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

SOURCES += main.cpp

RESOURCES += bars.qrc

OTHER_FILES += doc/src/* \
               doc/images/* \
               qml/bars/*
