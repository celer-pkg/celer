set_source_files_properties(Constants.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE true
)

qt_add_library(CarRendering STATIC)
qt6_add_qml_module(CarRendering
    URI "CarRendering"
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        Constants.qml
        DirectoryFontLoader.qml
)
