TEMPLATE = subdirs
CONFIG += debug_and_release ordered
SUBDIRS = \
    remoteobjects_server \
    simpleswitch \
    websockets

qtHaveModule(widgets) {
    SUBDIRS += \
        modelviewclient \
        modelviewserver
}

contains(QT_CONFIG, ssl): SUBDIRS += ssl

qtHaveModule(quick) {
    SUBDIRS += \
        clientapp

}
