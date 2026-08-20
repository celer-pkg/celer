!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

QT += widgets \
    quickwidgets

SOURCES += main.cpp \
    piegraph.cpp \
    piewidget.cpp

HEADERS += piegraph.h \
    piewidget.h

OTHER_FILES += doc/src/* \
    doc/images/*
