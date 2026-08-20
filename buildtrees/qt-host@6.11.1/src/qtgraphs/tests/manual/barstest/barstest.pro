!include( ../tests.pri ) {
    error( "Couldn't find the tests.pri file!" )
}

SOURCES += main.cpp chart.cpp
HEADERS += chart.h

RESOURCES += barstest.qrc

QT += widgets
