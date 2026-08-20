!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

CONFIG += qmltypes
QML_IMPORT_NAME = SurfaceGalleryExample
QML_IMPORT_MAJOR_VERSION = 1

QT += graphs

SOURCES += main.cpp \
           datasource.cpp
HEADERS += datasource.h

RESOURCES += surfacegallery.qrc

OTHER_FILES += doc/src/* \
               doc/images/* \
               qml/surfacegallery/*
