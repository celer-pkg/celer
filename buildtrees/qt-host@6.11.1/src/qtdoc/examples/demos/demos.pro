TEMPLATE = subdirs

qtHaveModule(quick) {
    SUBDIRS += \
        samegame \
        clocks \
        maroon

    qtHaveModule(quickcontrols2) {
        SUBDIRS += coffee \
                   colorpaletteclient \
                   calqlatr 


        android|ios: SUBDIRS += hangman
        qtHaveModule(quick3d):qtHaveModule(quick3dphysics) {
            SUBDIRS += dice
        }
    }

    qtHaveModule(network) {
        qtHaveModule(xml) {
            SUBDIRS += rssnews
        }
    }
}
