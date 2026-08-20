function(_qt_internal_gen_particle_shaders target resource_name)

    set(output_qrc "generated_${resource_name}.qrc")
    set(output_dir "${CMAKE_CURRENT_BINARY_DIR}/.rcc")

    _qt_internal_get_tool_wrapper_script_path(tool_wrapper)
    set(particleshadergen_command
        COMMAND
            "${tool_wrapper}"
            "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::particleshadergen>"
            -o "${output_dir}"
            -r "${output_qrc}"
    )

    add_custom_command(
        OUTPUT "${output_dir}/${output_qrc}"
        ${particleshadergen_command}
        DEPENDS
            $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::particleshadergen>
    )

    _qt_internal_quick3d_generate_resource_from_qrc(${target} ${resource_name})
endfunction()
