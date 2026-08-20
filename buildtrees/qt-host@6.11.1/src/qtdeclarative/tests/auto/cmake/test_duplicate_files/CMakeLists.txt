cmake_minimum_required(VERSION 3.16)
project(test_duplicate_files LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Test)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(main)

target_sources(main PRIVATE
    main.cpp
)

qt_add_qml_module(main
    URI MyAppUri
)

target_link_libraries(main
    PRIVATE
    Qt::Quick
    Qt::Test
)

qt_target_qml_sources(main
    QML_FILES
        Main.qml
    RESOURCES
        file.txt
)

if(run STREQUAL "case_qml_files")
    qt_target_qml_sources(main
        QML_FILES
            Main.qml
    )
elseif(run STREQUAL "case_resources")
    qt_target_qml_sources(main
        RESOURCES
            file.txt
    )
else()
    message(FATAL_ERROR "Unrecognized run=${run}")
endif()

# Check for duplicate entries in the PROPERTIES
qt_query_qml_module(main
    QML_FILES qml_files
    RESOURCES resources
)
function(check_list list file_regex)
    set(original_list "${${list}}")
    list(FILTER ${list} INCLUDE REGEX "${file_regex}")
    list(LENGTH ${list} list_length)
    if(list_length GREATER 1)
        set(message_file_prefix "    ")
        list(JOIN original_list
            "\n${message_file_prefix}"
            original_list_msg
        )
        message(FATAL_ERROR
            "FileSet ${list} contains duplicate files matching: ${file_regex}\n"
            "  ${list}:\n"
            "${message_file_prefix}${original_list_msg}"
        )
    endif()
endfunction()
check_list(qml_files "/Main\.qml$")
check_list(resources "/file.txt$")
