qt_add_executable(testApp WIN32 MACOSX_BUNDLE
    main.cpp
)
target_compile_definitions(testApp PRIVATE
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(testApp PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Quick
)

# Resources:
set(qml_resource_files
    "easing_rect_translate.json"
    "layer_effect_fill.json"
    "layer_effect_slider.json"
    "linked_layers.json"
    "main.qml"
    "path_and_ellipse.json"
    "rect_anchor_moved_static2.json"
    "rect_anchor_std_anim.json"
    "rect_anim_easing.json"
    "rect_gfill_linear.json"
    "rect_gfill_radial.json"
    "rect_move_bezier.json"
    "rect_negative_scale_anchors.json"
    "rect_negative_scale_pos.json"
    "rect_negative_scale_pos_anchors_rot.json"
    "rect_rotate.json"
    "rect_rotate_layer.json"
    "rect_rotate_layer2.json"
    "rect_rotate_shape.json"
    "rect_scale.json"
    "rect_scale_anchors_on_layer.json"
    "rect_scale_anchors_on_rect.json"
    "rect_skew.json"
    "rect_skew_axis.json"
    "repeater1.json"
    "repeater_group1.json"
    "repeater_group2.json"
    "shape_bezier_simple.json"
    "shape_circle.json"
    "shape_complex.json"
    "shape_complex2.json"
    "shape_static.json"
    "trim_path1.json"
    "trim_path2.json"
    "trim_path2_parallel.json"
    "trim_path2_offset.json"
    "trim_path_multiple1.json"
    "trim_path_multiple2.json"
    "trim_path_multiple3.json"
    "trim_path_multiple4.json"
    "trim_path_sequential2.json"
    "gradient_opacity_unpair.json"
)

qt_add_resources(testApp "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS testApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET testApp
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
