!include( ../tests.pri ) {
    error( "Couldn't find the tests.pri file!" )
}

QT += multimedia multimediawidgets

SOURCES += main.cpp \
    videodata.cpp

HEADERS += videodata.h

RESOURCES += qmlqtquick3dintegration.qrc
