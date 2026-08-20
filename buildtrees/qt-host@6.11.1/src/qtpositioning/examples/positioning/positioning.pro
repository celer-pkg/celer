TEMPLATE = subdirs

qtHaveModule(widgets): SUBDIRS += logfilepositionsource
qtHaveModule(quick) {
    SUBDIRS += satelliteinfo

    qtHaveModule(network): SUBDIRS += weatherinfo
}
