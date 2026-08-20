TARGET = places_list
TEMPLATE = app

QT += quick qml positioning location
SOURCES = main.cpp

RESOURCES += \
    places_list.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/location/places_list
INSTALLS += target

