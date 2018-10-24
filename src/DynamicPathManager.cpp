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

    _sectionSources.clear( );
    _synapseSources.clear( );
    _sources.clear( );
    _rootSources.clear( );

    _pendingSections.clear( );
    _pendingSources.clear( );
    _pendingSynapses.clear( );
  }


  void DynamicPathManager::synapse( unsigned long int synapsePtr )
  {
    nsolMSynapse_ptr synapse = reinterpret_cast< nsolMSynapse_ptr >( synapsePtr );

//    std::cout << "Reached synapse " << synapse << std::endl;

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
      source->functionType( TSF_ROOT );

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
            auto sectionSource = _sectionSources.find( childSectionID );
            if( sectionSource == _sectionSources.end( ))
            {
              auto source = _psManager->getSpareMobileSouce( );
              source->maxEmissionCycles( 1 );
              source->functionType( TSF_BIFURCATION );

              _createSourceOnDeepestPath( child->section( ), source );

              _sectionSources.insert( std::make_pair( childSectionID, source ));
            }
            else
            {
              if( sectionSource->second->active( ))
              {
                _orphanSources.insert( sectionSource->second );

                auto source = _psManager->getSpareMobileSouce( );
                source->maxEmissionCycles( 1 );
                source->functionType( TSF_BIFURCATION );

                _createSourceOnDeepestPath( child->section( ), source );

                _sectionSources.erase( childSectionID );
                _sectionSources.insert( std::make_pair( childSectionID, source ));

              }
              else
                sectionSource->second->restart( );
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
//      std::cout << "Creating source for synapse " << synapse->gid( ) << std::endl;
      auto path = _pathFinder->getPostsynapticPath( synapse );

      if( path.empty( ) && synapse->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC )
      {
        std::cout << "Error: Assigning empty post path to synapse " << synapse << std::endl;
        continue;
      }

      auto section = synapse->postSynapticSection( );
      if( !section )
        continue;

      if( synapse->synapseType( ) == nsol::MorphologySynapse::AXOSOMATIC )
        continue;

      auto synapseSource = _synapseSources.find( synapse );
      if( synapseSource == _synapseSources.end( ))
      {

        auto source = _psManager->getSpareMobileSouce( POSTSYNAPTIC );
        source->maxEmissionCycles( 1 );
        source->functionType( TSF_SYNAPSE );
        source->path( path );
        source->restart( );

        source->finishedPath.connect( boost::bind( &DynamicPathManager::finished, this, _1 ));

        _sources.insert( std::make_pair( source->gid( ), source ));
        _synapseSources.insert( std::make_pair( synapse, source ));
      }
      else
      {
        if( synapseSource->second->active( ))
        {
          _orphanSources.insert( synapseSource->second );

          auto source = _psManager->getSpareMobileSouce( POSTSYNAPTIC );
          source->maxEmissionCycles( 1 );
          source->functionType( TSF_SYNAPSE );
          source->path( path );
          source->restart( );

          source->finishedPath.connect( boost::bind( &DynamicPathManager::finished, this, _1 ));

          _sources.insert( std::make_pair( source->gid( ), source ));
          _synapseSources.erase( synapse );
          _synapseSources.insert( std::make_pair( synapse, source ));

        }
        else
          synapseSource->second->restart( );
      }
    }

    _pendingSynapses.clear( );
  }

  void DynamicPathManager::processFinishedPaths( void )
  {
//    std::cout << "Releashing finished sources" << std::endl;

    for( auto sourceID : _pendingSources )
    {
//      std::cout << "Processing source " << sourceID << std::endl;

      auto sourceIt = _sources.find( sourceID );
      if( sourceIt != _sources.end( ))
      {
        auto source = sourceIt->second;

        if( _rootSources.find( source ) == _rootSources.end( ))
        {
//          source->active( false );
//          std::cout << "Releasing source " << sourceID << std::endl;
//          _psManager->releaseMobileSource( source );
//          _sources.erase( sourceID );
          if( _orphanSources.find( source ) != _orphanSources.end( ))
          {
            _psManager->releaseMobileSource( source );
            _orphanSources.erase( source );
            _sources.erase( sourceID );
          }

        }
      }
      else
        std::cout << "ERROR: Source " << sourceID << " not found" << std::endl;

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

