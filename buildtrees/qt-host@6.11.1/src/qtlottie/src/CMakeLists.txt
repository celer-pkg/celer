set(QT_SBOM_DEFAULT_QT_LICENSE_ID_LIBRARIES "QT_COMMERCIAL_OR_GPL3")

qt_internal_add_qml_module(Lottie
    URI "Qt.labs.lottieqt"
    VERSION "${PROJECT_VERSION}"
    DEPENDENCIES
        QtQuick
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::Qml
        Qt::Quick
)

qt_internal_add_docs(Lottie
    qml/doc/qtlottieanimation.qdocconf
)

add_subdirectory(lottie)
add_subdirectory(qml)

add_subdirectory(vectorimagehelpers)

if(TARGET Qt::QuickPrivate
    AND TARGET Qt::QuickShapesPrivate
    AND TARGET Qt::QuickVectorImageGeneratorPrivate)
    add_subdirectory(lottiegenerator)
endif()
add_subdirectory(plugins)
