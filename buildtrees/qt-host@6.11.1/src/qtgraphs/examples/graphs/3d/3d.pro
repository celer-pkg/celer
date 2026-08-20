TEMPLATE = subdirs
qtHaveModule(quick) {
    SUBDIRS += bars \
               scatter \
               axishandling \
               surfacegallery
}

!android:!ios:!winrt {
    SUBDIRS += widgetgraphgallery \
               widgetvolumetric
}
