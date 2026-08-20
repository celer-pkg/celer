# For now, don't built auto tests when QT_BUILD_MINIMAL_STATIC_TEST
# is specified and the build is targeting iOS. QT_BUILD_MINIMAL_STATIC_TEST is used in our CI.
# Regular non-cmake build tests shouldn't be built because the CI will try to run them and fail
# due to missing simulator support.
if(IOS AND QT_BUILD_MINIMAL_STATIC_TESTS)
    return()
endif()

add_subdirectory(qshaderbaker)
add_subdirectory(buildtimeqsb)

_qt_internal_check_depfile_support(has_depfile_support)
if(has_depfile_support AND NOT CMAKE_CROSSCOMPILING)
    # The test is disabled for cross-compiling platforms since we cannot embed the test results.
    add_subdirectory(qsbdepfiles)
endif()
