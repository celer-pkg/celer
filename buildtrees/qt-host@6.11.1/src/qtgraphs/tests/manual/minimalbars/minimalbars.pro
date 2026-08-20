!include( ../tests.pri ) {
    error( "Couldn't find the tests.pri file!" )
}

QT += core gui graphs

TARGET = MinimalBars
TEMPLATE = app

SOURCES += ../../../src/graphs3d/doc/snippets/doc_src_q3dbars_construction.cpp
