/*
 * @file  DynamicPathManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "DynamicPathManager.h"

namespace syncopa
{
  DynamicPathManager::DynamicPathManager( )
  : _presynapticGID( 0 )
  , _pathFinder( nullptr )
  , _psManager( nullptr )
  { }

  void DynamicPathManager::init( PathFinder* pathFinder_, PSManager* psManager_ )
  {
    _pathFinder = pathFinder_;
    _psManager = psManager_;
  }


  void DynamicPathManager::_createSourceOnDeepestPath( nsolMSection_ptr origin,
                                                       MobilePolylineSource* source )
  {

    if( !_pathFinder )
      return;

    if( _sources.find( source->gid( )) != _sources.end( ))
      std::cout << "ERROR: Creating already existing source" << std::endl;

    auto computedPath = _pathFinder->computeDeepestPathFrom( origin->id( ));
    if( computedPath.size( ) > 0 )
    {
      _sources.insert( std::make_pair( source->gid( ), source ));
      source->path( computedPath );
    }
    else
    {
      std::cout << "Error: path from node " << origin->id( ) << " not found." << std::endl;
      return;
    }
//    std::cout << std::endl;

//    source->initialNode( origin );
    source->restart( );

    source->finishedPath.connect( boost::bind( &DynamicPathManager::finished, this, _1 ));

    source->finishedSection.connect( boost::bind( &DynamicPathManager::finishedSection, this, _1 ));

    source->reachedSynapse.connect( boost::bind( &DynamicPathManager::synapse, this, _1 ));
  }

  void DynamicPathManager::clear( void )
  {
    for( auto source : _sources )
      _psManager->releaseMobileSource( source.second );

    for( auto source: _rootSources )
      _psManager->releaseMobileSource( source );

    _sources.clear( );
    _rootSources.clear( );

    _pendingSections.clear( );
    _pendingSources.clear( );
    _pendingSynapses.clear( );
  }


  void DynamicPathManager::synapse( unsigned long int synapsePtr )
  {
    nsolMSynapse_ptr synapse = reinterpret_cast< nsolMSynapse_ptr >( synapsePtr );

    std::cout << "Reached synapse " << synapse << std::endl;

    _pendingSynapses.push_back( synapse );
  }

  void DynamicPathManager::finished( unsigned int sourceID )
  {

    _pendingSources.push_back( sourceID );


  }

  void DynamicPathManager::finishedSection( unsigned int sectionID )
  {
//    std::cout << "Reached end of section " << sectionID << std::endl;
    _pendingSections.push_back( sectionID );

  }

  void DynamicPathManager::createRootSources( void )
  {
    for( auto rootNode : _pathFinder->presynapticTree( ).rootNodes( ))
    {
//      auto section = rootNode->section( );
      auto source = _psManager->getSpareMobileSouce( );

      _createSourceOnDeepestPath( rootNode->section( ), source );

      std::cout << "Generating root node: " << rootNode->section( )->id( ) << std::endl;

      _rootSources.insert( source );
    }
  }

  void DynamicPathManager::processPendingSections( )
  {

    // Finished sections
    for( auto sectionID : _pendingSections )
    {

      auto node = _pathFinder->presynapticTree( ).node( sectionID );
      if( !node )
      {
        std::cout << "ERROR: Section node " << sectionID << " not found. " << std::endl;
        return;
      }

      {
        unsigned int nodeSectionID = node->deepestChild( )->section( )->id( );

        for( auto child : node->children( ))
        {
          unsigned int childSectionID = child->section( )->id( );

          if( childSectionID != nodeSectionID )
          {
//            auto sectionSource = _sources.find( childSectionID );
//            if( sectionSource != _sources.end( ))
//            {
//              sectionSource->second.second->restart( );
//            }
//            else
            {
              auto source = _psManager->getSpareMobileSouce( );
              source->maxEmissionCycles( 1 );

              _createSourceOnDeepestPath( child->section( ), source );
            }
          }
        }
      }
    }


    _pendingSections.clear( );

  }

  void DynamicPathManager::processPendingSynapses( void )
  {
//    return; //TODO
    for( auto synapse : _pendingSynapses )
    {
      auto path = _pathFinder->getPostsynapticPath( synapse );

      auto section = synapse->postSynapticSection( );
      if( !section )
        continue;

      auto source = _psManager->getSpareMobileSouce( );
      source->maxEmissionCycles( 1 );

      source->path( path );

      source->restart( );

      source->finishedPath.connect( boost::bind( &DynamicPathManager::finished, this, _1 ));

      _sources.insert( std::make_pair( source->gid( ), source ));
    }

    _pendingSynapses.clear( );
  }

  void DynamicPathManager::processFinishedPaths( void )
  {
    for( auto sourceID : _pendingSources )
    {
      auto sourceIt = _sources.find( sourceID );
      if( sourceIt != _sources.end( ))
      {
        auto source = sourceIt->second;

        if( _rootSources.find( source ) == _rootSources.end( ))
        {
          std::cout << "Releasing source " << sourceID << std::endl;
          _psManager->releaseMobileSource( source );
          _sources.erase( sourceID );
        }
      }
//      else
//        std::cout << "ERROR: Source " << sourceID << " not found" << std::endl;

    }

    _pendingSources.clear( );
  }

  void DynamicPathManager::restart( void )
  {
    for( auto source : _sources )
    {
      _psManager->releaseMobileSource( source.second );
    }
    _sources.clear( );

    for( auto source : _rootSources )
      source->restart( );
  }


}

