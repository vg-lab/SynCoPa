cmake_minimum_required(VERSION 3.25)

project(SynCoPa
  VERSION 0.4.1
  LANGUAGES CXX
  DESCRIPTION "SYNapses and COnnectivity PAths visualizer"
)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

# Prevent in-source builds
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# Include custom modules
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Options
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(BUILD_DOCS "Build documentation" ON)
# option(SYNCOPA_WITH_EXAMPLES "Build examples" ON)
# option(SYNCOPA_WITH_TESTS "Build tests" ON)
option(SYNCOPA_OPTIONALS_AS_REQUIRED "Force optional dependencies as required" OFF)

# Dependencies
# find_package(Boost CONFIG COMPONENTS unit_test_framework)
find_package(Boost CONFIG COMPONENTS thread REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL WebSockets REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Eigen3 REQUIRED NO_MODULE)
# find_package(Brion REQUIRED)
# find_package(neurolots REQUIRED)
# find_package(nsol REQUIRED)
# find_package(plab REQUIRED)
# find_package(ReTo REQUIRED)
# find_package(scoop REQUIRED)
# find_package(OpenMP) not used

include(FetchContent)
FetchContent_Declare(
    Brion
    GIT_REPOSITORY https://github.com/vg-lab/Brion.git
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Brion)

FetchContent_Declare(
    neurolots
    GIT_REPOSITORY https://github.com/vg-lab/neurolots.git
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(neurolots)

FetchContent_Declare(
    nsol
    GIT_REPOSITORY https://github.com/vg-lab/nsol.git
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nsol)

FetchContent_Declare(
    plab
    GIT_REPOSITORY https://github.com/vg-lab/particlelab.git
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(plab)

FetchContent_Declare(
    ReTo
    GIT_REPOSITORY https://github.com/vg-lab/ReTo.git
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(ReTo)

FetchContent_Declare(
    scoop
    GIT_REPOSITORY https://github.com/vg-lab/scoop.git
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(scoop)

# Optional dependencies
set(SYNCOPA_OPTS_FIND_ARGS "")
if(SYNCOPA_OPTIONALS_AS_REQUIRED)
  set(SYNCOPA_OPTS_FIND_ARGS REQUIRED)
endif()

find_package(OpenMP ${SYNCOPA_OPTS_FIND_ARGS} QUIET)
# find_package(Foo ${SYNCOPA_OPTS_FIND_ARGS} QUIET)

# Definitions
option(SYNCOPA_USE_GMRVLEX "" OFF)
# SYNCOPA_USE_BRION
# SYNCOPA_USE_PREFR
# SYNCOPA_USE_NEUROLOTS
# SYNCOPA_USE_NSOL
# SYNCOPA_USE_RETO
# SYNCOPA_USE_SCOOP
option(SYNCOPA_USE_BRION "" ON)

set(DEFAULT_CONTEXT_OPENGL_MAJOR 4)
set(DEFAULT_CONTEXT_OPENGL_MINOR 4)

# Add subdirectories
add_subdirectory(src)

# Define installation directory variables
include(GNUInstallDirs)

# Create an export
install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  # INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  # PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME_LOWER}
  # FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  # FILE_SET public_headers_generated DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# # Install the project targets file
# install(EXPORT ${PROJECT_NAME}Targets
#   FILE ${PROJECT_NAME_LOWER}-targets.cmake
#   NAMESPACE ${PROJECT_NAME}::
#   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME_LOWER}
# )

# # Include the CMake module that provides helpers for creating package config files
# include(CMakePackageConfigHelpers)

# # Generate the project-specific cmake config file
# configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/project-config.cmake.in
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config.cmake"
#   INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME_LOWER}
# )

# # Generate the project-specific cmake config version file
# write_basic_package_version_file(
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake"
#   VERSION ${PROJECT_VERSION}
#   COMPATIBILITY SameMajorVersion
# )

# # Install the cmake config files
# install(FILES
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config.cmake"
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake"
#   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME_LOWER}
# )

# # Install repo data
# install(FILES
#   "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
#   "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt"
#   DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME_LOWER}
# )

# # Allow the configured project-config.cmake in the build directory to be used by
# # other projects, without needing it to be installed
# export(EXPORT ${PROJECT_NAME}Targets
#   NAMESPACE ${PROJECT_NAME}::
#   FILE ${PROJECT_NAME_LOWER}-targets.cmake
# )

# Generate documentation
find_package(Doxygen)
if(DOXYGEN_FOUND AND BUILD_DOCS)
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs_doxygen)
  set(DOXYFILE_IN ${PROJECT_SOURCE_DIR}/cmake/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  doxygen_add_docs(
    ${PROJECT_NAME}_doxygen
    CONFIG_FILE ${DOXYFILE_OUT}
  )

  install(DIRECTORY
    ${DOXYGEN_OUTPUT_DIR}/html
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT ${PROJECT_NAME}_documentation
    OPTIONAL
  )
endif()

# Packaging
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
  /.git
  /.cache
  /build
  /.DS_Store
  /.idea
  /.vscode
)
include(CPack)
