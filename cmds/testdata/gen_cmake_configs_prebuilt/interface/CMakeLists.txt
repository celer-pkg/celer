cmake_minimum_required(VERSION 3.10.0)

project(tests VERSION 0.1.0 LANGUAGES C CXX)

add_executable(tests main.cpp)

find_package(FFmpeg REQUIRED)
target_link_libraries(tests PRIVATE FFmpeg::prebuilt-ffmpeg-interface)