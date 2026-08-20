TEMPLATE = subdirs

SUBDIRS += heartrate-server

qtHaveModule(widgets) {
    SUBDIRS += btchat
}

qtHaveModule(quick): SUBDIRS += lowenergyscanner \
                                heartrate-game
