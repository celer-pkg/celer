!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

DEFINES += QMAKE_BUILD

QT += datavisualization

SOURCES += main.cpp \
           datasource.cpp
HEADERS += datasource.h

RESOURCES += qmlsurfacegallery.qrc

OTHER_FILES += doc/src/* \
               doc/images/* \
               qml/qmlsurfacegallery/*
