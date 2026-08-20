qt_add_library(Ast_Venodhb STATIC)

qt_add_qml_module(Ast_Venodhb
    URI "Quick3DAssets.Venodhb_LOD0"
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        Venodhb_LOD0.qml
)
