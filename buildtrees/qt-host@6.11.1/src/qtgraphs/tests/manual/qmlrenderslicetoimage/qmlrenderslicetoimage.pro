!include( ../tests.pri ) {
    error( "Couldn't find the tests.pri file!" )
}

SOURCES += main.cpp

RESOURCES += qmlrenderslicetoimage.qrc

OTHER_FILES += qml/qmlrenderslicetoimage/*
