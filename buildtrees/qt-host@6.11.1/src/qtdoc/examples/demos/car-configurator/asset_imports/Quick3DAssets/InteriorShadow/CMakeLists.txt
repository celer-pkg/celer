qt_add_library(Ast_InteriorShadow STATIC)

qt6_add_qml_module(Ast_InteriorShadow
    URI "Quick3DAssets.InteriorShadow"
    CLASS_NAME "InteriorShadowPlugin"
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        InteriorShadow.qml
)
