TEMPLATE = subdirs

qtHaveModule(widgets) {
    SUBDIRS +=  trafficlight-widgets-static
    SUBDIRS +=  trafficlight-widgets-dynamic
    SUBDIRS +=  sudoku
}

qtHaveModule(quick) {
    SUBDIRS +=  calculator
    SUBDIRS +=  trafficlight-qml-static
    SUBDIRS +=  trafficlight-qml-dynamic
    SUBDIRS +=  trafficlight-qml-simple
    SUBDIRS +=  mediaplayer
    SUBDIRS +=  invoke
}

SUBDIRS += ftpclient
