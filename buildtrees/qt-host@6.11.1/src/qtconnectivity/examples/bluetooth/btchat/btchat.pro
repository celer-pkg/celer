TEMPLATE = app
TARGET = btchat

QT = core bluetooth widgets
requires(qtConfig(listwidget))

SOURCES = \
    main.cpp \
    chat.cpp \
    remoteselector.cpp \
    chatserver.cpp \
    chatclient.cpp

HEADERS = \
    chat.h \
    remoteselector.h \
    chatserver.h \
    chatclient.h

FORMS = \
    chat.ui \
    remoteselector.ui

theme_resources.files = \
    icons/btchat/24x24/bluetooth.png \
    icons/btchat/24x24/bluetooth_dark.png \
    icons/btchat/24x24/send.png \
    icons/btchat/24x24/send_dark.png \
    icons/btchat/24x24@2/bluetooth.png \
    icons/btchat/24x24@2/bluetooth_dark.png \
    icons/btchat/24x24@2/send.png \
    icons/btchat/24x24@2/send_dark.png \
    icons/btchat/24x24@3/bluetooth.png \
    icons/btchat/24x24@3/bluetooth_dark.png \
    icons/btchat/24x24@3/send.png \
    icons/btchat/24x24@3/send_dark.png \
    icons/btchat/24x24@4/bluetooth.png \
    icons/btchat/24x24@4/bluetooth_dark.png \
    icons/btchat/24x24@4/send.png \
    icons/btchat/24x24@4/send_dark.png \
    icons/btchat/index.theme

theme_resources.prefix = /

RESOURCES += theme_resources

ios: QMAKE_INFO_PLIST = ../shared/Info.qmake.ios.plist
macos: QMAKE_INFO_PLIST = ../shared/Info.qmake.macos.plist

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/btchat
INSTALLS += target
