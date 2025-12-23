cmake_minimum_required(VERSION 3.14)
project(my_lib VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MY_LIB_COMPONENTS component1 component2)
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# -- Create component1 -- #
add_library(component1 INTERFACE)
add_library(${PROJECT_NAME}::component1 ALIAS component1)
target_include_directories(component1 INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)
target_link_directories(component1 INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
  $<INSTALL_INTERFACE:lib>
)
target_link_libraries(${PROJECT_NAME} INTERFACE component1)

# -- Create component2 -- #
add_library(component2 INTERFACE)
add_library(${PROJECT_NAME}::component2 ALIAS component2)
target_include_directories(component2 INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)
target_link_directories(component2 INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
  $<INSTALL_INTERFACE:lib>
)
target_link_libraries(${PROJECT_NAME} INTERFACE component2)

# Install header files.
install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
  DESTINATION include/${PROJECT_NAME}
)

# Install library files.
file(GLOB_RECURSE LIB_FILES "${PROJECT_SOURCE_DIR}/lib/*.so*")
if (LIB_FILES)
  install(FILES ${LIB_FILES} DESTINATION lib)
endif()

# Install DLL and bin files.
file(GLOB_RECURSE BIN_FILES "${PROJECT_SOURCE_DIR}/bin/*")
if (BIN_FILES)
  install(FILES ${BIN_FILES} DESTINATION bin)
endif()

# Install components.
foreach(component ${MY_LIB_COMPONENTS})
  install(
    TARGETS ${component}
    EXPORT ${PROJECT_NAME}Targets
    INCLUDES DESTINATION include/${PROJECT_NAME}
  )
endforeach()

include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  INCLUDES DESTINATION include/${PROJECT_NAME}
)

install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION lib/cmake/${PROJECT_NAME}
)

install(
  FILES
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION
    lib/cmake/${PROJECT_NAME}
)