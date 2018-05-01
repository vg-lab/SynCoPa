# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   ViSimpl
#   2015-2016 (c) ViSimpl / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# ViSimpl project and version
cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

# streaminapp project and version
project( streaminapp VERSION 0.1.0 )
set( streaminapp_VERSION_ABI 1 )

# Disable in source building
if( "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" )
  message( FATAL "no in source building allowed.")
endif()

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common )

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/CMake/common/Common.cmake)
  message(FATAL_ERROR "CMake/common missing, run: git submodule update --init")
endif()

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug )
endif( NOT CMAKE_BUILD_TYPE )

if( CMAKE_BUILD_TYPE MATCHES "Debug" )
  add_definitions(-DDEBUG)
endif()

include(GitExternal)

# description
set(PREFREDITOR_DESCRIPTION "Streaming")
set(COMMON_PROJECT_DOMAIN gmrv.es)

include(Common)

common_find_package( Qt5Core 5.4 REQUIRED )
common_find_package( Qt5Gui 5.4 REQUIRED )
common_find_package( Qt5Widgets 5.4 REQUIRED )
common_find_package( Qt5OpenGL REQUIRED )
common_find_package( GLEW REQUIRED SYSTEM )
common_find_package( OpenGL REQUIRED )
common_find_package( Eigen3 REQUIRED SYSTEM)
common_find_package( ReTo REQUIRED )
common_find_package( prefr REQUIRED )
common_find_package( neurolots REQUIRED )
common_find_package( nsol REQUIRED )
common_find_package( Brion REQUIRED )
common_find_package( OpenMP )
common_find_package( gmrvlex )


list( APPEND PREFREDITOR_DEPENDENT_LIBRARIES
  OpenGL
  GLEW
  Eigen3
  ReTo
  prefr
  neurolots
  nsol
  Brion
)

if( QT5CORE_FOUND )
  list( APPEND PREFREDITOR_DEPENDENT_LIBRARIES Qt5Core )
endif( )

if( QT5WIDGETS_FOUND )
  list( APPEND PREFREDITOR_DEPENDENT_LIBRARIES Qt5Widgets )
endif( )

if( QT5OPENGL_FOUND )
  list( APPEND PREFREDITOR_DEPENDENT_LIBRARIES Qt5OpenGL )
endif( )

if( OPENMP_FOUND )
  list( APPEND PREFREDITOR_DEPENDENT_LIBRARIES OpenMP )
endif( )
if( GMRVLEX_FOUND )
  list( APPEND PREFREDITOR_DEPENDENT_LIBRARIES gmrvlex )
endif( )


common_find_package_post()

add_subdirectory( src )

include( CPackConfig )
include( DoxygenRule )
