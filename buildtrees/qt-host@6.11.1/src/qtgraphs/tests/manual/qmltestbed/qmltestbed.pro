!include( ../tests.pri ) {
    error( "Couldn't find the tests.pri file!" )
}

QT += core graphs gui qml quick

SOURCES += main.cpp \
    custombarseries.h custombarseries.cpp \
    customlineseries.h customlineseries.cpp \
    barsetgenerator.h barsetgenerator.cpp \
    barmodelmappermodel.h barmodelmappermodel.cpp \
    piemodelmappermodel.h piemodelmappermodel.cpp

RESOURCES += qmltestbed.qrc

OTHER_FILES += qml/qmltestbed/*
