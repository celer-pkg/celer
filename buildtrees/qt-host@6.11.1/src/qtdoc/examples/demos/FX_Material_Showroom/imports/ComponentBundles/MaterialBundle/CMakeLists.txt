qt_add_library(CB_MaterialBundle STATIC)
qt_add_qml_module(CB_MaterialBundle
    URI "ComponentBundles.MaterialBundle"
    CLASS_NAME CB_MaterialBundlePlugin
    QML_FILES
        AsphaltMaterial.qml
        CopperMaterial.qml
        GoldMaterial.qml
        SilverMaterial.qml
)
