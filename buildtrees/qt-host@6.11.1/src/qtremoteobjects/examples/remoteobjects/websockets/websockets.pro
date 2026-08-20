TEMPLATE = subdirs

qtHaveModule(widgets): qtHaveModule(websockets) {
    SUBDIRS += \
        wsclient \
        wsserver
}
