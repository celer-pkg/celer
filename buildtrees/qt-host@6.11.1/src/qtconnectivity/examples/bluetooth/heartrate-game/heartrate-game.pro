TEMPLATE = app
TARGET = heartrate-game

QT += qml quick bluetooth

CONFIG += qmltypes
QML_IMPORT_NAME = HeartRateGame
QML_IMPORT_MAJOR_VERSION = 1

HEADERS += \
    connectionhandler.h \
    deviceinfo.h \
    devicefinder.h \
    devicehandler.h \
    bluetoothbaseclass.h \
    heartrate-global.h

SOURCES += main.cpp \
    connectionhandler.cpp \
    deviceinfo.cpp \
    devicefinder.cpp \
    devicehandler.cpp \
    bluetoothbaseclass.cpp

qml_resources.files = \
    qmldir \
    App.qml \
    BluetoothAlarmDialog.qml \
    BottomLine.qml \
    Connect.qml \
    GameButton.qml \
    GamePage.qml \
    GameSettings.qml \
    Measure.qml \
    SplashScreen.qml \
    Stats.qml \
    StatsLabel.qml \
    TitleBar.qml \
    Main.qml \
    images/alert.svg \
    images/bluetooth.svg \
    images/bt_off_to_on.png \
    images/clock.svg \
    images/heart.png \
    images/logo.png \
    images/progress.svg \
    images/search.svg

qml_resources.prefix = /qt/qml/HeartRateGame

RESOURCES = qml_resources

ios: QMAKE_INFO_PLIST = ../shared/Info.qmake.ios.plist
macos: QMAKE_INFO_PLIST = ../shared/Info.qmake.macos.plist

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/heartrate-game
INSTALLS += target
