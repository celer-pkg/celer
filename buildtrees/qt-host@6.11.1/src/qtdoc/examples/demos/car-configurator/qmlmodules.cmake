add_subdirectory(asset_imports)
add_subdirectory(content)
add_subdirectory(imports)

target_link_libraries(carconfigurator PRIVATE
    CarConfiguratorContentplugin
    CarRenderingplugin
    Ast_SportsCarplugin
    Ast_InteriorShadowplugin
    Ast_LightDecalplugin
    Ast_Pebblesplugin
    Ast_ShadowPlaneplugin
    Ast_Venodhbplugin
    Ast_Ventdeeplugin
    Ast_Snowplugin
)
