TARGET = qml_location_geojsonviewer
TEMPLATE = app

QT += core qml network quick positioning
android: QT += core-private

SOURCES += main.cpp
CONFIG += install_ok

RESOURCES += \
    qml.qrc

OTHER_FILES += \
    $$files(data/*.json)

target.path = $$[QT_INSTALL_EXAMPLES]/location/geojsonviewer
INSTALLS += target
DEFINES += $$shell_quote(SRC_PATH=$$PWD)
