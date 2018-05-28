# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   ViSimpl
#   2015-2016 (c) ViSimpl / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

add_definitions(-DSTREAMINAPP_SKIP_GLEW_INCLUDE )

set(PROJECT_VERSION_ABI ${${PROJECT_NAME}_VERSION_ABI})

set(Name streaminapp)
set(NAME STREAMINAPP)
set(namespace streaminapp)
set(NAMESPACE STREAMINAPP)
set(INCLUDE_NAME streaminapp)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/api.h
  ${PROJECT_BINARY_DIR}/include/streaminapp/api.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.h
  ${PROJECT_BINARY_DIR}/include/streaminapp/version.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.cpp
  ${PROJECT_BINARY_DIR}/src/streaminapp/version.cpp @ONLY)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)


set(STREAMINAPP_SOURCES
  ${PROJECT_BINARY_DIR}/src/streaminapp/version.cpp
  streaminApp.cpp
  MainWindow.cpp
  OpenGLWidget.cpp
  
  NeuronScene.cpp
  PSManager.cpp
  PathFinder.cpp
  
  prefr/SourceMultiPosition.cpp
  prefr/UpdaterStaticPosition.cpp
  
  CMakeSetup.rc
  streaminApp.ui
  resources.qrc
)

set(STREAMINAPP_HEADERS
  ${PROJECT_BINARY_DIR}/include/streaminapp/version.h
  OpenGLWidget.h
  MainWindow.h
  
  NeuronScene.h
  PSManager.h
  PathFinder.h
  
  prefr/SourceMultiPosition.h
  prefr/UpdaterStaticPosition.h
  
  prefr/PrefrShaders.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_BINARY_DIR}/include ${PROJECT_BINARY_DIR}/src)

set(STREAMINAPP_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
  ReTo
  prefr
  nsol
  Brion
  Brain
  nlgeometry
  nlrender
  nlgenerator
)

if ( GMRVLEX_FOUND )
  LIST(APPEND STREAMINAPP_LINK_LIBRARIES gmrvlex )
endif( )

if ( NOT DEFAULT_CONTEXT_OPENGL_MAJOR )
  set( DEFAULT_CONTEXT_OPENGL_MAJOR 4 )
endif( )
if ( NOT DEFAULT_CONTEXT_OPENGL_MINOR )
  set( DEFAULT_CONTEXT_OPENGL_MINOR 0 )
endif( )

add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MAJOR=${DEFAULT_CONTEXT_OPENGL_MAJOR}" )
add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MINOR=${DEFAULT_CONTEXT_OPENGL_MINOR}" )

common_application( streaminapp GUI ${COMMON_APP_ARGS})
