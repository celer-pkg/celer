android|ios|winrt {
    error( "This example is not supported for android, ios, or winrt." )
}

SOURCES += main.cpp scatterdatamodifier.cpp
HEADERS += scatterdatamodifier.h

QT += widgets graphs

RESOURCES += rotations.qrc

OTHER_FILES += doc/src/* \
               doc/images/*
