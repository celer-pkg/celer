if (EMSCRIPTEN AND QT_FEATURE_thread)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_subdirectory(csv-parser)
