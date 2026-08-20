TEMPLATE = subdirs

qtHaveModule(quick): qtHaveModule(svg): {
    SUBDIRS += sensorsshowcase
}
