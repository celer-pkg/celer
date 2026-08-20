# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# We can't create the same interface imported target multiple times, CMake will complain if we do
# that. This can happen if the find_package call is done in multiple different subdirectories.
if(TARGET WrapIconv::WrapIconv)
    set(WrapIconv_FOUND ON)
    return()
endif()

include(CheckCXXSourceCompiles)

set(iconv_test_sources "#include <iconv.h>

int main(int, char **)
{
    iconv_t x = iconv_open(\"\", \"\");
    iconv_close(x);
    return 0;
}")

check_cxx_source_compiles("${iconv_test_sources}" HAVE_ICONV)
if(NOT HAVE_ICONV)
    set(_req_libraries "${CMAKE_REQUIRE_LIBRARIES}")
    set(CMAKE_REQUIRE_LIBRARIES "iconv")
    check_cxx_source_compiles("${iconv_test_sources}" HAVE_ICONV_WITH_LIB)
    set(CMAKE_REQUIRE_LIBRARIES "${_req_libraries}")
endif()

add_library(WrapIconv::WrapIconv INTERFACE IMPORTED)
if(HAVE_ICONV_WITH_LIB)
    target_link_libraries(WrapIconv::WrapIconv INTERFACE iconv)
endif()

set(WrapIconv_FOUND 1)
