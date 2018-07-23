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
  , _maxDepth( 0)
  { }

  void DynamicPathManager::init( PathFinder* pathFinder_, PSManager* psManager_ )
  {
    _pathFinder = pathFinder_;
    _psManager = psManager_;
  }

  void DynamicPathManager::configure( unsigned int presynapticGid,
                                      const std::vector< nsol::MorphologySynapsePtr >& synapses,
                                      const tSectionsInfoMap& infoSections )
  {
    clear( );

    _infoSections = infoSections;

    _presynapticGID = presynapticGid;

    unsigned int maxDepth = 0;

    for( auto syn : synapses )
    {
      auto section =
          dynamic_cast< nsol::NeuronMorphologySectionPtr >( syn->preSynapticSection( ));

      if( !section )
        continue;

      auto infoSection = _infoSections.find( section );
      if( infoSection == _infoSections.end( ))
      {
        std::cout << "Section info not found " << section << std::endl;
        continue;
      }

      // TODO Add post paths from synapses

      if( !_treePath.hasNode( section ))
      {
        unsigned int depth = _treePath.addBranch( section, infoSection->second );

        if( depth > maxDepth )
          maxDepth = depth;
      }
    }

    _maxDepth = maxDepth;

    for( auto rootNode : _treePath.rootNodes( ))
    {
//      auto section = rootNode->section( );
      auto source = _psManager->getSpareMobileSouce( );

      _createSourceOnDeepestPath( rootNode, source );

      _rootSources.insert( source );
    }

    _treePath.print( );

  }


  void DynamicPathManager::_createSourceOnDeepestPath( cnode_ptr origin,
                                                       MobilePolylineSource* source )
  {

    utils::EventPolylineInterpolation interpolator;

    auto computedPath = _computedPaths.find( origin->section( )->id( ));
    if( computedPath == _computedPaths.end( ))
    {

      std::vector< ConnectivityNode* > pathSections;
      pathSections.reserve( origin->childrenMaxDepth( ) + 1);
      pathSections.push_back( origin );

      auto firstPath = origin->deepestPath( );

      for( auto node : firstPath )
      {
        pathSections.push_back( node );
      }

      std::cout << "Found deepest path with " << pathSections.size( )
                << " out of " << origin->childrenMaxDepth( )
                << std::endl;

      _sources.insert( std::make_pair( source->gid( ), source ));

      auto transform = _pathFinder->getTransform( _presynapticGID );

  //      tPosVec nodes;

  //    std::cout << "NOdes " << pathNodes.size( );
      cnode_ptr last = nullptr;
      for( auto currentSection : pathSections )
      {
  //        std::cout << " " << connectivityNode->section( )->id( )
  //                  << ":" << interpolator.totalDistance( )
  //                  << "->" << ( connectivityNode->numberOfChildren( ) )
  //                  << std::endl;

        float prevDist = interpolator.totalDistance( );

        std::cout << "-------------" << std::endl;
        for( auto node : currentSection->section( )->nodes( ))
        {
          auto point = transformPoint( node->point( ), transform );

          std::cout << point << std::endl;

          interpolator.insert( point );
        }
  //        for( auto nodeIt = sec->nodes( ).rbegin( ); nodeIt != sec->nodes( ).rend( ); ++nodeIt )
  //          nodes.push_back( transformPoint( ( *nodeIt )->point( ), transform ));

        if( last && last->numberOfChildren( ) > 1 )
        {
          auto children = last->children( );
  //          for( auto child = children.begin( ) + 1; child != children.end( ); ++child )
          {
            interpolator.addEventNode( prevDist, last->section( )->id() );
          }
        }

        currentSection->sectionLength( interpolator.totalDistance( ) - prevDist );

        last = currentSection;
      }

      _computedPaths.insert( std::make_pair( origin->section( )->id( ), interpolator ));
    }
    else
      interpolator = computedPath->second;
//    std::cout << std::endl;

    source->path( interpolator );

    source->initialNode( origin );
    source->restart( );

    source->finishedPath.connect( boost::bind( &DynamicPathManager::finished, this, _1 ));

    source->finishedSection.connect( boost::bind( &DynamicPathManager::finishedSection, this, _1 ));

  }


  void DynamicPathManager::addPostsynapticPath( nsol::MorphologySynapsePtr synapse,
                                                const tPosVec& nodes )
  {
    utils::PolylineInterpolation interpolator( nodes );

    _pathsPost.insert( std::make_pair( synapse, interpolator ));
  }


  void DynamicPathManager::clear( void )
  {
    for( auto source : _sources )
         _psManager->releaseMobileSource( source.second );

    _sources.clear( );

    _computedPaths.clear( );
    _treePath.clear( );
    _rootSources.clear( );
    _pathsPost.clear( );

    _infoSections.clear( );

    _pendingSections.clear( );

    _maxDepth = 0;
  }


  void DynamicPathManager::synapse( nsol::MorphologySynapsePtr  )
  {

  }

  void DynamicPathManager::finished( unsigned int sourceID )
  {

    _pendingSources.push_back( sourceID );


  }

  void DynamicPathManager::finishedSection( unsigned int sectionID )
  {
    std::cout << "Reached end of section " << sectionID << std::endl;
    _pendingSections.push_back( sectionID );

  }

  void DynamicPathManager::processPendingSections( )
  {

    // Finished sections
    for( auto sectionID : _pendingSections )
    {

      auto node = _treePath.node( sectionID );
      if( !node )
      {
        std::cout << "ERROR: Section node " << sectionID << " not found. " << std::endl;
        return;
      }

      {
        for( auto child : node->children( ))
        {
          unsigned int childSectionID = child->section( )->id( );

          if( childSectionID != node->deepestChild( )->section( )->id( ))
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

              _createSourceOnDeepestPath( child, source );
            }
          }
        }
      }
    }


    _pendingSections.clear( );

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
          _psManager->releaseMobileSource( source );
          _sources.erase( sourceID );
        }
      }

    }

    _pendingSources.clear( );
  }


}

