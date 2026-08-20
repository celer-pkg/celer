!include( ../tests.pri ) {
    error( "Couldn't find the tests.pri file!" )
}

QT += core gui graphs

TARGET = minimalSurface
TEMPLATE = app

SOURCES += ../../../src/graphs3d/doc/snippets/doc_src_q3dsurface_construction.cpp
