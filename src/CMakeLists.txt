# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   SynCoPa
#   2018 (c) SynCoPa / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

add_definitions(-DSYNCOPA_SKIP_GLEW_INCLUDE )
add_definitions(-DPLAB_SKIP_GLEW_INCLUDE )

set(PROJECT_VERSION_ABI ${${PROJECT_NAME}_VERSION_ABI})

set(Name syncopa)
set(NAME SYNCOPA)
set(namespace syncopa)
set(NAMESPACE SYNCOPA)
set(INCLUDE_NAME syncopa)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/api.h
  ${PROJECT_BINARY_DIR}/include/syncopa/api.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.h
  ${PROJECT_BINARY_DIR}/include/syncopa/version.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.cpp
  ${PROJECT_BINARY_DIR}/src/syncopa/version.cpp @ONLY)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)


set(SYNCOPA_SOURCES
  ${PROJECT_BINARY_DIR}/src/syncopa/version.cpp
  syncopa.cpp
  MainWindow.cpp
  OpenGLWidget.cpp
  PaletteColorWidget.cpp
  GradientWidget.cpp
  DomainManager.cpp

  NeuronScene.cpp
  ParticleManager.cpp
  PathFinder.cpp
  DynamicPathGenerator.cpp
  ConnectivityTree.cpp
  #DynamicPathManager.cpp
  SynCoPaWebAPI.cpp
  SynCoPaWebSocket.h
  SynCoPaWebSocket.cpp

  NeuronCluster.cpp
  NeuronClusterManager.cpp
  NeuronClusterView.cpp
  NeuronMetadata.cpp
  NeuronClusterViewTitle.cpp
  NeuronSelectionView.cpp
  TripleStateButton.cpp

  DataExport.cpp

  particlelab/StaticModel.cpp
  particlelab/StaticGradientModel.cpp
  particlelab/DynamicModel.cpp
  particlelab/SynapseParticle.cpp
  particlelab/DynamicPathParticle.cpp

  ext/ctkrangeslider.cpp

  CMakeSetup.rc
  syncopa.ui
  resources.qrc
)

set(SYNCOPA_HEADERS
  ${PROJECT_BINARY_DIR}/include/syncopa/version.h
  OpenGLWidget.h
  MainWindow.h
  PaletteColorWidget.h
  GradientWidget.h
  DomainManager.h

  NeuronScene.h
  ParticleManager.h
  PathFinder.h
  DynamicPathGenerator.h
  ConnectivityTree.h
  #DynamicPathManager.h
  SynCoPaWebAPI.h

  NeuronCluster.h
  NeuronClusterManager.h
  NeuronMetadata.h
  NeuronClusterView.h
  NeuronClusterViewTitle.h
  NeuronSelectionView.h
  TripleStateButton.h

  DataExport.h

  particlelab/ParticleLabShaders.h
  particlelab/StaticModel.h
  particlelab/StaticGradientModel.h
  particlelab/DynamicModel.h
  particlelab/SynapseParticle.h
  particlelab/DynamicPathParticle.h

  ext/ctkrangeslider.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_BINARY_DIR}/include ${PROJECT_BINARY_DIR}/src)

set(SYNCOPA_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::WebSockets
  Qt5::WebChannel
  ReTo
  plab
  nsol
  Brion
  Brain
  nlgeometry
  nlrender
  nlgenerator
  scoop
  boost_thread
)

if ( GMRVLEX_FOUND )
  LIST(APPEND SYNCOPA_LINK_LIBRARIES gmrvlex )
endif( )

if ( NOT DEFAULT_CONTEXT_OPENGL_MAJOR )
  set( DEFAULT_CONTEXT_OPENGL_MAJOR 4 )
endif( )
if ( NOT DEFAULT_CONTEXT_OPENGL_MINOR )
  set( DEFAULT_CONTEXT_OPENGL_MINOR 4 )
endif( )

add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MAJOR=${DEFAULT_CONTEXT_OPENGL_MAJOR}" )
add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MINOR=${DEFAULT_CONTEXT_OPENGL_MINOR}" )

common_application( syncopa GUI ${COMMON_APP_ARGS})
