TEMPLATE = app

QT += qml scxml

SOURCES += trafficlight-qml-dynamic.cpp

QML_IMPORT_NAME = TrafficLightApplication
QML_IMPORT_MAJOR_VERSION = 1

qml_resources.files = \
    qmldir \
    MainView.qml \
    Button.qml \
    Lights.ui.qml \
    ../trafficlight-common/statemachine.scxml \
    ../trafficlight-common/play.png \
    ../trafficlight-common/yellow.png \
    ../trafficlight-common/red.png \
    ../trafficlight-common/green.png \
    ../trafficlight-common/background.png \
    ../trafficlight-common/pause.png \

qml_resources.prefix = /qt/qml/TrafficLightApplication

RESOURCES += qml_resources

target.path = $$[QT_INSTALL_EXAMPLES]/scxml/trafficlight-qml-dynamic
INSTALLS += target
