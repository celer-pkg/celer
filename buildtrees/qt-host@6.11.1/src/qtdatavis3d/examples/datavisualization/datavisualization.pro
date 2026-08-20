TEMPLATE = subdirs
qtHaveModule(quick) {
    SUBDIRS += qmlbars \
               qmlscatter \
               qmlaxishandling \
               qmlsurfacegallery
}

!android:!ios:!winrt {
    SUBDIRS += graphgallery \
               volumetric
}

