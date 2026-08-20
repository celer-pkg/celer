android|winrt {
    error( "This example is not supported for android or winrt." )
}

!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

SOURCES += main.cpp volumetric.cpp
HEADERS += volumetric.h

QT += widgets

OTHER_FILES += doc/src/* \
               doc/images/*

RESOURCES += widgetvolumetric.qrc
