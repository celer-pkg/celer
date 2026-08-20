TEMPLATE = subdirs

SUBDIRS = \
    simple

qtHaveModule(gui): qtHaveModule(concurrent) {
    SUBDIRS += colorpalette
}
