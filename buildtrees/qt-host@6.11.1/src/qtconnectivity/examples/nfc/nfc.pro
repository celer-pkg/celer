TEMPLATE = subdirs
qtHaveModule(widgets) {
    SUBDIRS += annotatedurl
}
qtHaveModule(quickcontrols2) {
     SUBDIRS += ndefeditor
}
