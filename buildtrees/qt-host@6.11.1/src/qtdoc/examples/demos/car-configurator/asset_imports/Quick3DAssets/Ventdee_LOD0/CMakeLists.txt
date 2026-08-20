qt_add_library(Ast_Ventdee STATIC)

qt_add_qml_module(Ast_Ventdee
    URI "Quick3DAssets.Ventdee_LOD0"
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        Ventdee_LOD0.qml
)
