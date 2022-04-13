/*
 * @file  visimpl.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "types.h"

#include <QApplication>
#include "MainWindow.h"
#include <QDebug>

#include <syncopa/version.h>

void setFormat( void );
void usageMessage(  char* progName );
void dumpVersion( void );
bool atLeastTwo( bool a, bool b, bool c );

int main( int argc, char** argv )
{
  // Linux osg obj importer has a bug with non english lang.
#ifndef Win32
  setenv("LANG", "C", 1);
#endif

  QApplication application( argc, argv );

  bool fullscreen = false, initWindowSize = false, initWindowMaximized = false,
    updateOnIdle = false, fps = false;
  int initWindowWidth = 0, initWindowHeight = 0;

  std::string blueConfig, target;

  for( int i = 1; i < argc; i++ )
  {
    if ( std::strcmp( argv[i], "--help" ) == 0 ||
         std::strcmp( argv[i], "-h" ) == 0 )
    {
      usageMessage( argv[0] );
      return 0;
    }
    if ( std::strcmp( argv[i], "--version" ) == 0 )
    {
      dumpVersion( );
      return 0;
    }

    if( std::strcmp( argv[ i ], "-bc") == 0 )
    {
      if( i + 1 <= argc )
      {
        blueConfig = argv[ ++i ];
      }
      else
        usageMessage( argv[ 0 ]);
    }

    if( strcmp( argv[ i ], "-target") == 0 )
    {
      if( i + 1 <= argc )
      {
        target = argv[ ++i ];
      }
      else
        usageMessage( argv[ 0 ]);
    }

    if ( strcmp( argv[i], "--fullscreen" ) == 0 ||
         strcmp( argv[i],"-fs") == 0 )
    {
      fullscreen = true;
    }
    if ( strcmp( argv[i], "--maximize-window" ) == 0 ||
         strcmp( argv[i],"-mw") == 0 )
    {
      initWindowMaximized = true;
    }
    if ( strcmp( argv[i], "--window-size" ) == 0 ||
         strcmp( argv[i],"-ws") == 0 )
    {
      initWindowSize = true;
      if ( i + 2 >= argc )
        usageMessage( argv[0] );
      initWindowWidth = atoi( argv[ ++i ] );
      initWindowHeight = atoi( argv[ ++i ] );

    }
    if ( strcmp( argv[i], "--frames-per-second" ) == 0 ||
         strcmp( argv[i],"-fps") == 0 )
    {
      fps = true;
    }
    if ( strcmp( argv[i], "--update-on-idle" ) == 0 ||
         strcmp( argv[i],"-uoi") == 0 )
    {
      updateOnIdle = true;
    }
  }

  std::cout << "Loading BlueConfig " << blueConfig
            << " target " << target
            << std::endl;

  setFormat( );
  // Showing FPS needs update on idle activated
  MainWindow mainWindow( nullptr , fps || updateOnIdle , fps );
  mainWindow.setWindowTitle("SynCoPa");

  if ( initWindowSize )
    mainWindow.resize( initWindowWidth, initWindowHeight );
  else if ( initWindowMaximized )
    mainWindow.showMaximized( );
  else if ( fullscreen )
    mainWindow.showFullScreen( );
  else
	  mainWindow.show( );

  mainWindow.init( );

  if( !blueConfig.empty( ) && !target.empty( ))
    mainWindow.loadData( blueConfig, target );

  return application.exec();
}

void usageMessage( char* progName )
{
  std::cerr << std::endl
            << "Usage: "
            << progName << std::endl
            << "\t[ -ws | --window-size ] width height ]"
            << std::endl
            << "\t[ -fs | --fullscreen ] "
            << std::endl
            << "\t[ -mw | --maximize-window ]"
            << std::endl
            << "\t[ -zeq <session*> ]"
            << std::endl
            << "\t[ --version ]"
            << std::endl
            << "\t[ --help | -h ]"
            << std::endl << std::endl
            << "\t* session: ZeroEQ session identifier. For example hbp://"
            << std::endl;

  exit(-1);
}

void dumpVersion( void )
{
  std::cerr << std::endl
            << "SynCoPa - SYNapses and COnnectivity PAths visualizer"
            << syncopa::Version::getMajor( ) << "."
            << syncopa::Version::getMinor( ) << "."
            << syncopa::Version::getPatch( )
            << " (" << syncopa::Version::getRevision( ) << ")"
            << std::endl << std::endl;
}

void setFormat( void )
{
  int ctxOpenGLMajor = DEFAULT_CONTEXT_OPENGL_MAJOR;
  int ctxOpenGLMinor = DEFAULT_CONTEXT_OPENGL_MINOR;
  int ctxOpenGLVSync = 1;
  int ctxOpenGLSamples = 16;

  if ( std::getenv("CONTEXT_OPENGL_MAJOR"))
    ctxOpenGLMajor = std::stoi( std::getenv("CONTEXT_OPENGL_MAJOR"));

  if ( std::getenv("CONTEXT_OPENGL_MINOR"))
    ctxOpenGLMinor = std::stoi( std::getenv("CONTEXT_OPENGL_MINOR"));

  if ( std::getenv("CONTEXT_OPENGL_SAMPLES"))
    ctxOpenGLSamples = std::stoi( std::getenv("CONTEXT_OPENGL_SAMPLES"));

  if ( std::getenv("CONTEXT_OPENGL_VSYNC"))
    ctxOpenGLVSync = std::stoi( std::getenv("CONTEXT_OPENGL_VSYNC"));

  std::cerr << "Setting OpenGL context to "
            << ctxOpenGLMajor << "." << ctxOpenGLMinor << std::endl;

  QSurfaceFormat format;
  format.setVersion( ctxOpenGLMajor, ctxOpenGLMinor);
  format.setProfile( QSurfaceFormat::CoreProfile );

  if ( ctxOpenGLSamples != 0 )
    format.setSamples( ctxOpenGLSamples );

  format.setSwapInterval( ctxOpenGLVSync );

  QSurfaceFormat::setDefaultFormat( format );
  if ( std::getenv("CONTEXT_OPENGL_COMPATIBILITY_PROFILE"))
    format.setProfile( QSurfaceFormat::CompatibilityProfile );
  else
    format.setProfile( QSurfaceFormat::CoreProfile );
}
