TEMPLATE = subdirs

qtHaveModule(widgets) {
    SUBDIRS += \
        redditclient
}
