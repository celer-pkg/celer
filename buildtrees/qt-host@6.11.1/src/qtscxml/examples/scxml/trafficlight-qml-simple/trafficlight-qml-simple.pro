TEMPLATE = app

QT += qml scxml
CONFIG += c++11
CONFIG += qmltypes

SOURCES += trafficlight-qml-simple.cpp

HEADERS += trafficlight-qml.h

STATECHARTS = ../trafficlight-common/statemachine.scxml

QML_IMPORT_NAME = TrafficLightApplication
QML_IMPORT_MAJOR_VERSION = 1

qml_resources.files = \
    qmldir \
    MainView.qml \
    Light.qml \
    ../trafficlight-common/play.png \
    ../trafficlight-common/yellow.png \
    ../trafficlight-common/red.png \
    ../trafficlight-common/green.png \
    ../trafficlight-common/background.png \
    ../trafficlight-common/pause.png \

qml_resources.prefix = /qt/qml/TrafficLightApplication

RESOURCES += qml_resources

target.path = $$[QT_INSTALL_EXAMPLES]/scxml/trafficlight-qml-simple
INSTALLS += target
