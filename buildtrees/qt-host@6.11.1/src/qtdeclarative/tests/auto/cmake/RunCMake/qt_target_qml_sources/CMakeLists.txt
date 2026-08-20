cmake_minimum_required(VERSION 3.16)
project(${RunCMake_TEST} LANGUAGES CXX)

# For this particular case, we want to make sure there is no Qt6 package in the root directory.
if(NOT RunCMake_TEST STREQUAL "subdirectory_without_qt_in_root")
    find_package(Qt6 REQUIRED COMPONENTS Qml Test)
endif()

include(${RunCMake_TEST}.cmake)
