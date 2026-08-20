android|winrt {
    error( "This example is not supported for android or winrt." )
}

!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

QT += widgets

requires(qtConfig(fontcombobox))
requires(qtConfig(combobox))

SOURCES += main.cpp \
    bargraph.cpp \
    graphmodifier.cpp \
    rainfalldata.cpp \
    scattergraph.cpp \
    scatterdatamodifier.cpp \
    surfacegraph.cpp \
    surfacegraphmodifier.cpp \
    highlightseries.cpp \
    topographicseries.cpp \
    scattergraphwidget.cpp \
    surfacegraphwidget.cpp

HEADERS += \
    bargraph.h \
    graphmodifier.h \
    rainfalldata.h \
    scattergraph.h \
    scatterdatamodifier.h \
    surfacegraph.h \
    surfacegraphmodifier.h \
    highlightseries.h \
    topographicseries.h \
    scattergraphwidget.h \
    surfacegraphwidget.h

RESOURCES += widgetgraphgallery.qrc

OTHER_FILES += doc/src/* \
               doc/images/* \
               data/*
