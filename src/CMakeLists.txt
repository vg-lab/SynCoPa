# Define public headers
set(SYNCOPA_PUBLIC_HEADERS
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
  # DynamicPathManager.h
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

# Define source files
set(SYNCOPA_SOURCES
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
  # DynamicPathManager.cpp
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

# # Create the includes strings
# set(HEADER_INCLUDES "")
# foreach(header ${SYNCOPA_PUBLIC_HEADERS})
#   string(APPEND HEADER_INCLUDES "#include <${PROJECT_NAME_LOWER}/${header}>\n")
# endforeach()

# # Generate the unified header
# configure_file(
#   "${PROJECT_SOURCE_DIR}/cmake/unified_header.h.in"
#   "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}.h"
# )

# Get the current Git commit for the version header
find_package(Git)
if(Git_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} --no-pager log -n 1 HEAD "--pretty=format:%h"
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    RESULT_VARIABLE git_result
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(git_result EQUAL 0)
    message(STATUS "Last ${PROJECT_NAME} Git commit: ${GIT_COMMIT_HASH}")
  endif()
endif()

# Generate the version header
configure_file (
  "${PROJECT_SOURCE_DIR}/cmake/version.h.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWER}/version.h"
)

# Enable Qt generators
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Create the target
add_executable(${PROJECT_NAME})

# set_target_properties(${PROJECT_NAME} PROPERTIES
#   VERSION ${PROJECT_VERSION}
#   SOVERSION ${PROJECT_VERSION_MAJOR}
# )

# Add sources to the target
target_sources(${PROJECT_NAME}
  PRIVATE
    ${SYNCOPA_SOURCES}
  PRIVATE
    FILE_SET public_headers
      TYPE HEADERS
      BASE_DIRS
        ${PROJECT_SOURCE_DIR}
      FILES
        ${SYNCOPA_PUBLIC_HEADERS}
)

# Add generated sources to the target
target_sources(${PROJECT_NAME}
  PRIVATE
    FILE_SET public_headers_generated
      TYPE HEADERS
      BASE_DIRS
        ${PROJECT_BINARY_DIR}
      FILES
        # Generated headers
        # ${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWER}/${PROJECT_NAME_LOWER}.h
        # ${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWER}/${PROJECT_NAME_LOWER}_export.h
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWER}/version.h
)

# Add compile definitions 
target_compile_definitions(${PROJECT_NAME}
  PUBLIC
  PRIVATE
    DEFAULT_CONTEXT_OPENGL_MAJOR=${DEFAULT_CONTEXT_OPENGL_MAJOR}
    DEFAULT_CONTEXT_OPENGL_MINOR=${DEFAULT_CONTEXT_OPENGL_MINOR}
    $<$<BOOL:${SYNCOPA_USE_BRION}>:SYNCOPA_USE_BRION>
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Boost::thread
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::WebSockets
    neurolots::nlgeometry
    neurolots::nlrender
    neurolots::nlgenerator
    nsol::nsol
    plab::plab
    ReTo::ReTo
    scoop::scoop
  PRIVATE
    Brion
    Brain
    "$<$<BOOL:${OpenMP_FOUND}>:OpenMP::OpenMP_CXX>"
)
