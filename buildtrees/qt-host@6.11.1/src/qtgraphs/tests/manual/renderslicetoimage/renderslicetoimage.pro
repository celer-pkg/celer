android|ios|winrt {
    error( "This example is not supported for android, ios, or winrt." )
}

SOURCES += main.cpp surfacegraph.cpp surfacegraphmodifier.cpp surfacegraphwidget.cpp
HEADERS += surfacegraph.h surfacegraphmodifier.h surfacegraphwidget.h

QT += widgets graphs

RESOURCES += renderslicetoimage.qrc
