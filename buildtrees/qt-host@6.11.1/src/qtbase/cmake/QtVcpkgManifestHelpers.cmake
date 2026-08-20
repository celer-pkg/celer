# CMake API for generating vcpkg.json manifest files

# Initialize the internal vcpkg manifest data with the given JSON string.
#
# This function is called internally by qt_vcpkg_manifest_init and may be used to set the internal
# JSON data to a JSON string read from a file.
function(qt_vcpkg_set_internal_manifest_data json)
    set_property(GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON "${json}")
endfunction()

# Initialize a new vcpkg manifest with basic package information.
#
# This function creates the foundation for a vcpkg.json manifest file by setting up
# the basic structure with package name and version. It must be called before any
# other qt_vcpkg_* functions.
#
# Arguments:
#   NAME - Required. The package name for the vcpkg manifest
#   VERSION - Optional. The package version string
#   BUILTIN_BASELINE - Optional. Baseline for version resolution.
function(qt_vcpkg_manifest_init)
    set(no_value_options "")
    set(single_value_options
        BUILTIN_BASELINE
        NAME
        VERSION
    )
    set(multi_value_options "")
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "${no_value_options}" "${single_value_options}" "${multi_value_options}"
    )
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT DEFINED arg_NAME)
        message(FATAL_ERROR "qt_vcpkg_manifest_init: NAME is required")
    endif()

    string(JSON manifest_json SET "{}" name "\"${arg_NAME}\"")
    string(JSON manifest_json SET "${manifest_json}" dependencies "[]")
    string(JSON manifest_json SET "${manifest_json}" features "{}")
    string(JSON manifest_json SET "${manifest_json}" default-features "[]")
    if(DEFINED arg_VERSION)
        string(JSON manifest_json SET "${manifest_json}" version "\"${arg_VERSION}\"")
    endif()
    if(DEFINED arg_BUILTIN_BASELINE)
        string(JSON manifest_json SET "${manifest_json}" builtin-baseline
            "\"${arg_BUILTIN_BASELINE}\""
        )
    endif()

    qt_vcpkg_set_internal_manifest_data("${manifest_json}")
endfunction()

# Add a dependency to the vcpkg manifest.
#
# This function adds either a simple string dependency or a complex dependency object
# to the manifest's dependencies array or to a specific feature's dependencies.
#
# Arguments:
#   dependency - Required. The name of the package dependency
#   VERSION - Optional. Specific version constraint for the dependency
#   PLATFORM - Optional. Platform expression to limit when dependency is used
#   FEATURES - Optional. List of features to enable for this dependency
#   DEFAULT_FEATURES - Optional. Boolean to set default-features.
#   ADD_TO_FEATURE - Optional. Name of feature to add this dependency to. If not specified,
#                    adds to the manifest's root dependencies array.
#
# If VERSION, PLATFORM, FEATURES, or DEFAULT_FEATURES are specified, a complex dependency object is
# created. When FEATURES is specified, "default-features": false is automatically set. Otherwise, a
# simple string dependency is added.
#
# If ADD_TO_FEATURE is specified but the feature doesn't exist, it will be created with a default
# description.
function(qt_vcpkg_add_dependency name)
    set(no_value_options "")
    set(single_value_options
        ADD_TO_FEATURE
        DEFAULT_FEATURES
        PLATFORM
        VERSION
    )
    set(multi_value_options
        FEATURES
    )
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "${no_value_options}" "${single_value_options}" "${multi_value_options}"
    )
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    get_property(manifest_json GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON)
    if(NOT manifest_json)
        message(FATAL_ERROR "qt_vcpkg_add_dependency: Must call qt_vcpkg_manifest_init first")
    endif()

    if(arg_ADD_TO_FEATURE)
        # Check if feature exists, create with default description if not
        string(JSON feature_obj ERROR_VARIABLE feature_error GET "${manifest_json}" features
            ${arg_ADD_TO_FEATURE}
        )
        if(feature_error)
            string(JSON feature_obj SET "{}" description "\"Use ${arg_ADD_TO_FEATURE}\"")
        endif()

        string(JSON deps_exists ERROR_VARIABLE deps_error GET "${feature_obj}" dependencies)
        if(deps_error)
            string(JSON feature_obj SET "${feature_obj}" dependencies "[]")
        endif()

        set(target_obj "${feature_obj}")
        set(target_path "features" "${arg_ADD_TO_FEATURE}")
    else()
        set(target_obj "${manifest_json}")
        set(target_path "")
    endif()

    string(JSON deps_length LENGTH "${target_obj}" dependencies)
    if(DEFINED arg_VERSION OR DEFINED arg_FEATURES OR DEFINED arg_PLATFORM
        OR DEFINED arg_DEFAULT_FEATURES)
        string(JSON dep_obj SET "{}" name "\"${name}\"")

        if(arg_VERSION)
            string(JSON dep_obj SET "${dep_obj}" version>= "\"${arg_VERSION}\"")
        endif()

        if(arg_FEATURES)
            if(NOT DEFINED arg_DEFAULT_FEATURES)
                set(arg_DEFAULT_FEATURES OFF)
            endif()
            string(JSON dep_obj SET "${dep_obj}" features "[]")
            set(feature_index 0)
            foreach(feature ${arg_FEATURES})
                string(JSON dep_obj SET "${dep_obj}" features ${feature_index} "\"${feature}\"")
                math(EXPR feature_index "${feature_index} + 1")
            endforeach()
        endif()

        if(DEFINED arg_DEFAULT_FEATURES)
            if(arg_DEFAULT_FEATURES)
                string(JSON dep_obj SET "${dep_obj}" default-features true)
            else()
                string(JSON dep_obj SET "${dep_obj}" default-features false)
            endif()
        endif()

        if(arg_PLATFORM)
            string(JSON dep_obj SET "${dep_obj}" platform "\"${arg_PLATFORM}\"")
        endif()

        string(JSON target_obj SET "${target_obj}" dependencies ${deps_length} "${dep_obj}")
    else()
        string(JSON target_obj SET "${target_obj}" dependencies ${deps_length} "\"${name}\"")
    endif()

    if(arg_ADD_TO_FEATURE)
        string(JSON manifest_json SET "${manifest_json}" features ${arg_ADD_TO_FEATURE}
            "${target_obj}"
        )
    else()
        set(manifest_json "${target_obj}")
    endif()

    set_property(GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON "${manifest_json}")
endfunction()

# Add a feature definition to the vcpkg manifest.
#
# This function creates or updates a basic feature in the manifest with the specified
# description and metadata. It does not modify feature dependencies - use the dedicated
# dependency functions for that purpose.
#
# Arguments:
#   name - Required. The feature name
#   description - Required. Human-readable description of the feature
#   DEFAULT - Optional flag. If set, feature is added to default-features
#   SUPPORTS - Optional. Platform expression defining where feature is supported
#
# The function preserves existing dependencies when updating an existing feature.
function(qt_vcpkg_feature name description)
    set(no_value_options DEFAULT)
    set(single_value_options SUPPORTS)
    set(multi_value_options "")
    cmake_parse_arguments(PARSE_ARGV 2 arg
        "${no_value_options}" "${single_value_options}" "${multi_value_options}"
    )
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    get_property(manifest_json GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON)
    if(NOT manifest_json)
        message(FATAL_ERROR "qt_vcpkg_feature: Must call qt_vcpkg_manifest_init first")
    endif()

    string(JSON feature_obj ERROR_VARIABLE feature_error GET "${manifest_json}" features ${name})
    if(feature_error)
        set(feature_obj "{}")
    endif()
    string(JSON feature_obj SET "${feature_obj}" description "\"${description}\"")

    if(arg_SUPPORTS)
        string(JSON feature_obj SET "${feature_obj}" supports "\"${arg_SUPPORTS}\"")
    endif()

    string(JSON manifest_json SET "${manifest_json}" features ${name} "${feature_obj}")

    if(arg_DEFAULT)
        string(JSON default_features_length LENGTH "${manifest_json}" default-features)
        string(JSON manifest_json SET "${manifest_json}" default-features ${default_features_length}
            "\"${name}\""
        )
    endif()

    set_property(GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON "${manifest_json}")
endfunction()

# Add feature dependencies to an existing feature.
#
# This function creates self-referential dependencies where one feature in a package
# depends on other features in the same package.
#
# Arguments:
#   feature_name - Required. Name of the feature to modify
#   dependency_features - Required. List of features this feature depends on
#
# Creates a dependency object with the package's own name and the specified features,
# with "default-features": false to only enable the specified features.
function(qt_vcpkg_add_feature_dependencies feature_name)
    set(dependency_features ${ARGN})

    get_property(manifest_json GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON)
    if(NOT manifest_json)
        message(FATAL_ERROR
            "qt_vcpkg_add_feature_dependencies: Must call qt_vcpkg_manifest_init first"
        )
    endif()

    string(JSON package_name GET "${manifest_json}" name)
    qt_vcpkg_add_dependency(${package_name} ADD_TO_FEATURE ${feature_name}
        FEATURES ${dependency_features}
    )
endfunction()

# Write the vcpkg manifest to a file.
#
# This function serializes the current manifest JSON structure to the specified
# output file.
#
# Arguments:
#   output_file - Required. Path where the vcpkg.json manifest should be written
#
# The function removes empty top-level elements from the manifest before writing
# to avoid cluttering the output file.
function(qt_vcpkg_write_manifest output_file)
    get_property(manifest_json GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON)
    if(NOT manifest_json)
        message(FATAL_ERROR "qt_vcpkg_write_manifest: Must call qt_vcpkg_manifest_init first")
    endif()

    # Remove empty elements
    foreach(element_name IN ITEMS dependencies features default-features)
        string(JSON element_length LENGTH "${manifest_json}" "${element_name}")
        if(element_length EQUAL 0)
            string(JSON manifest_json REMOVE "${manifest_json}" "${element_name}")
        endif()
    endforeach()

    file(CONFIGURE OUTPUT "${output_file}" CONTENT "${manifest_json}")
    message(STATUS "Generated vcpkg manifest: ${output_file}")
endfunction()

# Return the default-features of the current manifest in out_var.
#
# This function reads the default-features from the internal manifest data and
# writes their names to out_var.
function(qt_vcpkg_get_default_features out_var)
    get_property(manifest_json GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON)
    if(NOT manifest_json)
        message(FATAL_ERROR "qt_vcpkg_get_default_features: Must call qt_vcpkg_manifest_init first")
    endif()

    set("${out_var}" "" PARENT_SCOPE)
    string(JSON default_features ERROR_VARIABLE json_error GET "${manifest_json}" default-features)
    if(json_error)
        return()
    endif()
    string(JSON default_features_length ERROR_VARIABLE json_error LENGTH "${default_features}")
    if(json_error)
        return()
    endif()

    set(result "")
    math(EXPR max_i "${default_features_length} - 1")
    foreach(i RANGE 0 ${max_i})
        string(JSON element ERROR_VARIABLE json_error GET "${default_features}" "${i}")
        if(json_error)
            continue()
        endif()
        string(JSON element_type ERROR_VARIABLE json_error TYPE "${default_features}" "${i}")
        if(json_error)
            continue()
        endif()

        set(feature "")
        if(element_type STREQUAL "STRING")
            set(feature "${element}")
        elseif(element_type STREQUAL "OBJECT")
            string(JSON feature ERROR_VARIABLE json_error GET "${element}" "name")
            if(json_error)
                continue()
            endif()
        endif()

        list(APPEND result "${feature}")
    endforeach()

    set("${out_var}" "${result}" PARENT_SCOPE)
endfunction()

# Return the names of all features in the current manifest in out_var.
#
# This function reads the features map from the current manifest and returns its keys
# (the feature names) in out_var.
function(qt_vcpkg_get_features out_var)
    get_property(manifest_json GLOBAL PROPERTY _QT_VCPKG_MANIFEST_JSON)
    if(NOT manifest_json)
        message(FATAL_ERROR "qt_vcpkg_get_features: Must call qt_vcpkg_manifest_init first")
    endif()

    set("${out_var}" "" PARENT_SCOPE)
    string(JSON features_obj ERROR_VARIABLE json_error GET "${manifest_json}" features)
    if(json_error)
        return()
    endif()
    string(JSON features_length ERROR_VARIABLE json_error LENGTH "${features_obj}")
    if(json_error)
        return()
    endif()

    set(result "")
    math(EXPR max_i "${features_length} - 1")
    foreach(i RANGE 0 ${max_i})
        string(JSON feature ERROR_VARIABLE json_error MEMBER "${features_obj}" ${i})
        if(json_error)
            continue()
        endif()
        list(APPEND result "${feature}")
    endforeach()

    set("${out_var}" "${result}" PARENT_SCOPE)
endfunction()
