  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderPath.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/shaderPath.h )

  include_directories( ${CMAKE_CURRENT_BINARY_DIR} )


set( extra_HEADERS )
set( extra_SOURCES
      SourceMultiPosition.cpp
      UpdaterStaticPosition.cpp
      MobilePolylineSource.cpp
      ../ConnectivityTree.cpp
      ../PathFinder.cpp
  )

  set( STATICPREFR_HEADERS )
  set( STATICPREFR_SOURCES
      prefrStatic.cpp
      ${extra_SOURCES}
  )
  
  set( STATICPREFR_LINK_LIBRARIES prefr nsol )
  common_application( staticPReFr )

  set( RENDERTOTEXTURE_HEADERS )
  set( RENDERTOTEXTURE_SOURCES renderToTexture.cpp )
  set( RENDERTOTEXTURE_LINK_LIBRARIES prefr nlgeometry nlrender nlgenerator )
  common_application( renderToTexture )

  set( BRANCHING_HEADERS )
  set( BRANCHING_SOURCES 
       branching.cpp
       ${extra_SOURCES}
  )
  set( BRANCHING_LINK_LIBRARIES prefr nlgeometry nlrender nlgenerator Brion Brain )
  common_application( branching )
  
  set( BRIONTEST_HEADERS )
  set( BRIONTEST_SOURCES projectionTest.cpp )
  set( BRIONTEST_LINK_LIBRARIES Brion Brain )
  common_application( brionTest )