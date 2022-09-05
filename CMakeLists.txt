# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   SynCoPa
#   2015-2016 (c) SynCoPa / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# SynCoPa project and version
cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

# syncopa project and version
project( syncopa VERSION 0.3.2 )
set( syncopa_VERSION_ABI 1 )
set( SYNCOPA_LICENSE "GPL")

# to make configuration messages cleaner, use -Wno-dev
cmake_policy(SET CMP0054 OLD)
cmake_policy(SET CMP0048 OLD)

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
set(SYNCOPA_DESCRIPTION "SynCoPa - SYNapses and COnnectivity PAths visualizer")
set(COMMON_PROJECT_DOMAIN gmrv.es)

include(Common)

common_find_package( Qt5Core 5.4 REQUIRED )
common_find_package( Qt5Gui 5.4 REQUIRED )
common_find_package( Qt5Widgets 5.4 REQUIRED )
common_find_package( Qt5OpenGL REQUIRED )
common_find_package( Qt5WebSockets REQUIRED )
common_find_package( Qt5WebChannel REQUIRED )
common_find_package( GLEW REQUIRED SYSTEM )
common_find_package( OpenGL REQUIRED )
common_find_package( Eigen3 REQUIRED SYSTEM)
common_find_package( neurolots REQUIRED )
common_find_package( ReTo REQUIRED )
common_find_package( nsol REQUIRED )
common_find_package( plab REQUIRED )
common_find_package( Brion REQUIRED )
common_find_package( OpenMP )
common_find_package( scoop REQUIRED )
common_find_package( wcw REQUIRED )

list( APPEND SYNCOPA_DEPENDENT_LIBRARIES
  plab
  ReTo
  neurolots
  nsol
  OpenGL
  GLEW
  Eigen3
  Brion
  wcw
)

if( QT5CORE_FOUND )
  list( APPEND SYNCOPA_DEPENDENT_LIBRARIES Qt5Core )
endif( )

if( QT5GUI_FOUND )
  list( APPEND SYNCOPA_DEPENDENT_LIBRARIES Qt5Gui )
endif( )

if( QT5WIDGETS_FOUND )
  list( APPEND SYNCOPA_DEPENDENT_LIBRARIES Qt5Widgets )
endif( )

if( QT5OPENGL_FOUND )
  list( APPEND SYNCOPA_DEPENDENT_LIBRARIES Qt5OpenGL )
endif( )

if( QT5WEBSOCKETS_FOUND )
  list( APPEND SYNCOPA_DEPENDENT_LIBRARIES Qt5WebSockets )
endif( )

if( QT5WEBCHANNEL_FOUND )
  list( APPEND SYNCOPA_DEPENDENT_LIBRARIES Qt5WebChannel )
endif( )

if( OPENMP_FOUND )
  list( APPEND SYNCOPA_DEPENDENT_LIBRARIES OpenMP )
endif( )

if( GMRVLEX_FOUND )
  list( APPEND SYNCOPA_DEPENDENT_LIBRARIES gmrvlex )
endif( )


common_find_package_post()

add_subdirectory( src )

include( CPackConfig )
include( DoxygenRule )
