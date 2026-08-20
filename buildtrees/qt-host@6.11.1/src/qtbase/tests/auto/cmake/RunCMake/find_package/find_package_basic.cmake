find_package(Qt6 REQUIRED COMPONENTS Core)

qt_add_library(foo)
target_sources(foo PRIVATE dummy.cpp)
target_link_libraries(foo PRIVATE Qt6::Core)
