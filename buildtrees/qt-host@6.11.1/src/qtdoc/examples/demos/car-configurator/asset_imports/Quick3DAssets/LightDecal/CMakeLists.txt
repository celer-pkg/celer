qt_add_library(Ast_LightDecal STATIC)

qt_add_qml_module(Ast_LightDecal
    URI "Quick3DAssets.LightDecal"
    CLASS_NAME "LightDecalPlugin"
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        LightDecal.qml
)
