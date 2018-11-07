/*
 * @file	PathFinder.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "PathFinder.h"

#include <brion/brion.h>
#include <brain/brain.h>

namespace syncopa
{

  PathFinder::PathFinder( void)
  : _dataset( nullptr )
  , _synapseFixInfo( nullptr )
  , _presynapticGID( 0 )
  , _maxDepth( 0 )
  { }

  PathFinder::~PathFinder( void )
  {

  }

  void PathFinder::dataset( nsol::DataSet* dataset_,
                            const TSynapseInfo* synapseInfo  )
  {
    _dataset = dataset_;
    _synapseFixInfo = synapseInfo;
  }

  void PathFinder::configure( unsigned int presynapticGid,
                              const gidUSet& postsynapticGIDs,
                              const tsynapseVec& synapses  )
  {
    clear( );

    std::cout << "Configuring for presynaptic " << presynapticGid
              << " and post ";
    for( auto gid : postsynapticGIDs )
      std::cout << gid << " ";
    std::cout << std::endl;

    _populateTrees( synapses );

    _processSections( synapses, presynapticGid, postsynapticGIDs );

    _processEndSections( presynapticGid, postsynapticGIDs );

    _presynapticGID = presynapticGid;


//    _treePre.print( );

  }

  void PathFinder::_populateTrees( const tsynapseVec& synapses )
  {

    for( auto syn : synapses )
    {
      auto sectionPre = syn->preSynapticSection( );
      auto sectionPost = syn->postSynapticSection( );


      if( !sectionPre )
      {
        std::cout << "ERROR: Unassigned synapse pre section " << syn->gid( )
                  << " " << syn->preSynapticNeuron( )
                  << "-" << syn->postSynapticNeuron( )
                  << std::endl;
        continue;
      }

      if( !sectionPost && syn->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC )
      {
        std::cout << "ERROR: Unassigned synapse post section " << syn->gid( )
                  << " " << syn->preSynapticNeuron( )
                  << "-" << syn->postSynapticNeuron( )
                  << std::endl;
        continue;
      }

//      auto infoSection = _infoSections.find( section );
//      if( infoSection == _infoSections.end( ))
//      {
//        std::cout << "Section info not found " << section << std::endl;
//        continue;
//      }

      // TODO Add post paths from synapses

      if( !_treePre.hasNode( sectionPre ))
      {
        auto sections = pathToSoma( sectionPre );

        _treePre.addBranch( sections );
      }

      auto treePost = _treePost.find( syn->postSynapticNeuron( ));
      if( treePost == _treePost.end( ))
      {
        auto tree = ConnectivityTree( );
        treePost =
            _treePost.insert( std::make_pair( syn->postSynapticNeuron( ), tree )).first;

      }

      if( !treePost->second.hasNode( sectionPost ))
      {
        auto postSections = pathToSoma( sectionPost );

        treePost->second.addBranch( postSections );
      }
    }
  }

  const ConnectivityTree& PathFinder::presynapticTree( void ) const
  {
    return _treePre;
  }


  void PathFinder::clear( void )
  {

    _computedPaths.clear( );
    _treePre.clear( );
    _treePost.clear( );

    _pathsPost.clear( );

    _infoSections.clear( );

    _maxDepth = 0;
  }



  std::vector< vec3 > PathFinder::getAllPathsPoints( unsigned int gid,
                                                     const gidUSet& gidsPost,
                                                     const tsynapseVec& synapses,
                                                     float pointSize,
                                                     TNeuronConnection type ) const
  {

    unsigned int currentGid = gid;
    vec3 synapsePos;

    std::set< unsigned int > gids = { gid };

//    auto synapseSections = processSections( synapses, currentGid, gidsPost );
//    auto synapseSections = parseSections( synapses, type );

    std::unordered_set< nsol::NeuronMorphologySectionPtr > insertedSections;

    std::vector< vec3 > sectionPoints;

    std::vector< mat4 > transforms;
    std::vector< nsol::Nodes > presynapticPaths;
    std::vector< nsol::Nodes > postsynapticPaths;

    std::vector< vec3 > result;

    for( auto syn : synapses )
    {
      if( !gidsPost.empty( ) && gidsPost.find( syn->postSynapticNeuron( )) == gidsPost.end( ))
      {
        std::cout << "ERROR: synapse post neuron " << syn->postSynapticNeuron( )
                  << " not found in gid set with size" << gidsPost.size( ) << std::endl;
        continue;
      }

      if( type == POSTSYNAPTIC )
      {
        currentGid = syn->postSynapticNeuron( );
        synapsePos = syn->postSynapticSurfacePosition( );
      }
      else
        synapsePos = syn->preSynapticSurfacePosition( );


      auto transform = getTransform( currentGid );

      auto sectionNodes = pathToSoma( syn, type );

      unsigned int counter = 0;
      for( auto section : sectionNodes )
      {
        if( insertedSections.find( section ) != insertedSections.end( ))
        {
          ++counter;
          continue;
        }


        insertedSections.insert( section );
//        std::cout << "Processing section with " << section->nodes( ).size( ) << " nodes." << std::endl;

        nsol::Nodes& nodes = section->nodes( );

        std::vector< vec3 > positions;

        utils::PolylineInterpolation pathPoints;

        auto parsedSection = _infoSections.find( section );
        if( parsedSection != _infoSections.end( ) &&
            std::get< tsi_leafSection >( parsedSection->second ))
        {
          auto points = std::get< tsi_fixedSection >( parsedSection->second );
          std::cout << "Fixed section with " << points.size( )
                    << " instead of " << nodes.size( ) << std::endl;

          pathPoints.insert( points );
//            accDist = pathPoints.totalDistance( );

//            std::cout << "Fixed section for synapse " << syn->preSynapticNeuron( ) << "-" << syn->postSynapticNeuron( )
//                      << " with " << points.size( ) << " nodes of " << nodes.size( ) <<  std::endl;

        }
        else
        {
          for( auto node : nodes )
            pathPoints.insert( transformPoint( node->point( ), transform ));

//            accDist = pathPoints.totalDistance( );
        }


        // Sample nodes with pointSize-separated points
        float currentDist = 0;
        float accDist = pathPoints.totalDistance( );
//          for( unsigned int i = 0; i < pathPoints.size( ); ++i )
        while( currentDist < accDist )
        {
          vec3 pos = pathPoints.pointAtDistance( currentDist );

//            vec3 pos = pathPoints[ i ];
          result.push_back( pos );

          currentDist += pointSize;
        } // while

        ++counter;
      } // for section

    } // for synapses

    return result;

  }


  std::vector< nsol::NeuronMorphologySectionPtr >
  PathFinder::pathToSoma( const nsolMSynapse_ptr synapse,
                              syncopa::TNeuronConnection type ) const
  {

    nsol::Nodes result;

    nsol::NeuronMorphologySectionPtr section = nullptr;

    if( type == PRESYNAPTIC )
    {
      section = dynamic_cast< nsolMSection_ptr >( synapse->preSynapticSection( ));
    }
    else
    {
      section = dynamic_cast< nsolMSection_ptr >( synapse->postSynapticSection( ));
    }

    return pathToSoma( section );

  }

  std::vector< nsolMSection_ptr > PathFinder::pathToSoma( nsolMSection_ptr section ) const
  {

    std::vector< nsol::NeuronMorphologySection* > sections;

    while( section )
    {
      sections.push_back( section );

      section = dynamic_cast< nsol::NeuronMorphologySection* >( section->parent( ));
    }

    return sections;
  }

  mat4 PathFinder::getTransform( unsigned int gid ) const
  {
    nsol::NeuronsMap& neurons = _dataset->neurons();
    auto neuron = neurons.find( gid );
    if( neuron == neurons.end( ))
      return mat4::Ones( );

    return neuron->second->transform( );
  }


  std::vector< vec3 > PathFinder::_cutEndSection( const std::vector< vec3 >& nodes,
                                                 const vec3& synapsePos,
                                                 unsigned int lastIndex ) const
  {
    std::vector< vec3 > result;

    for( unsigned int i = 0; i < lastIndex + 1; ++i )
    {
      result.push_back( nodes[ i ]);
    }

    result.push_back( synapsePos );

//    result = nodes;

    return result;
  }

  void
  PathFinder::_processSections( const std::vector< nsolMSynapse_ptr >& synapses,
                               unsigned int gidPre,
                               const gidUSet& postNeuronGIDs )
  {
    _infoSections.clear( );
    _somaSynapses.clear( );

    auto transformPre = getTransform( gidPre );

    unsigned int neuronGID = 0;

    for( auto synapse : synapses )
    {
      auto synInfo = _synapseFixInfo->find( synapse );
      if( synInfo == _synapseFixInfo->end( ))
      {
        std::cout << "ERROR: Synapse " << synapse->gid( ) << " info NOT FOUND" << std::endl;
        continue;
      }

      unsigned int gidPost = synapse->postSynapticNeuron( );

      if( !postNeuronGIDs.empty( ) &&
          postNeuronGIDs.find( gidPost ) == postNeuronGIDs.end( ))
      {
        std::cout << "ERROR: Postsynaptic GID " << gidPost
                  << " not found in postynaptic gid set with size "
                  << postNeuronGIDs.size( ) << std::endl;

        continue;
      }

      if( !synapse->preSynapticSection( ) ||
          ( !synapse->postSynapticSection( ) &&
              synapse->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC ))
      {
        std::cout << "ERROR: Synapse " << synapse->gid( ) << " section NOT FOUND" << std::endl;
        continue;
      }

      auto transformPost = getTransform( gidPost );

      for( unsigned int i = 0; i < ( unsigned int ) ALL_CONNECTIONS; ++i )
      {
        auto type = ( TNeuronConnection ) i;

        mat4 transform;
        if( type == PRESYNAPTIC )
        {
          neuronGID = gidPre;
          transform = transformPre;
        }
        else
        {
          neuronGID = gidPost;
          transform = transformPost;
        }

        const auto& synapseFixInfo = ( type == PRESYNAPTIC )
                                     ? std::get< TBSI_PRESYNAPTIC >( synInfo->second )
                                     : std::get< TBSI_POSTSYNAPTIC >( synInfo->second );

        nsolMSection_ptr section =
            ( type == PRESYNAPTIC ) ? synapse->preSynapticSection( )
                                    : synapse->postSynapticSection( );


        if( type == POSTSYNAPTIC && !section &&
            synapse->synapseType( ) == nsol::MorphologySynapse::AXOSOMATIC )
        {
          _somaSynapses.insert( synapse );
          std::cout << "Found somatic synapse " << synapse->gid( ) << std::endl;
          continue;
        }

        if( std::get< TBS_SECTION_ID >( synapseFixInfo ) != section->id( ))
        {
          std::cout << "ERROR: Section " << section->id( )
                    << " does not match " << std::get< TBS_SECTION_ID >( synapseFixInfo )
                    << std::endl;
          continue;
        }

        unsigned int segmentIndex = std::get< TBS_SEGMENT_INDEX >( synapseFixInfo );
        const auto& nodes = section->nodes( );

        if( nodes.empty( ) || nodes.size( ) == 1)
        {
          std::cout << "ERROR: Section " << section->id( )
                    << " with " << nodes.size( )
                    << " nodes." << std::endl;
          continue;
        }

        if( segmentIndex >= nodes.size( ) - 1)
        {
          std::cout << "ERROR: " << neuronGID << " Segment index " << segmentIndex
                    << " is greater than section " << section->id( )
                    << " nodes number " << nodes.size( )
                    << " distance " << std::get< TBS_SEGMENT_DISTANCE >( synapseFixInfo )
                    << std::endl;
          continue;
        }

        float distance = std::get< TBS_SEGMENT_DISTANCE >( synapseFixInfo );
        vec3 start = nodes[ segmentIndex ]->point( );
        vec3 end = nodes[ segmentIndex + 1 ]->point( );

        float length = ( end - start ).norm( );

        float normalized = distance / length;

        vec3 synapsePos = transformPoint( end * normalized + start * ( 1 - normalized ), transform );

        auto it = _infoSections.find( section );
        if( it == _infoSections.end( ))
        {
          utils::PolylineInterpolation interpolator;

          for( auto node : section->nodes( ))
            interpolator.insert( transformPoint( node->point( ), transform ));

          std::unordered_map< nsol::MorphologySynapsePtr,
                              tFixedSynapseInfo> synapseSet;

          float sectionDist = interpolator.totalDistance( );

          std::vector< vec3 > fixedSection;

          tSectionInfo data =
              std::make_tuple( synapseSet, sectionDist,
                               false,
                               fixedSection, interpolator );

          it = _infoSections.insert( std::make_pair( section, data )).first;

        } // if section not found

        auto& synapseMap = std::get< tsi_Synapses >( it->second );
        auto& interpolator = std::get< tsi_Interpolator >( it->second );
        float composedDistance = interpolator.distance( segmentIndex );

        composedDistance += distance;

        tFixedSynapseInfo fixedSynInfo =
            std::make_tuple( synapsePos, composedDistance, 0, segmentIndex, true );

        synapseMap.insert( std::make_pair( synapse, fixedSynInfo ));

      } // for presynaptic and postsynaptic

    } // for each synapse


  }

  void PathFinder::_processEndSections( unsigned int gidPre,
                                        const gidUSet& gidsPost )
  {
    gidUSet gids = gidsPost;
    gids.insert( gidPre );

    for( auto gid : gids )
    {
      const ConnectivityTree* tree = nullptr;

      if( gid == gidPre )
      {
        tree = &_treePre;
      }
      else
      {
        auto treeIt = _treePost.find( gid );
        if( treeIt == _treePost.end( ))
        {
          std::cout << "ERROR: Tree for neuron " << gid << " NOT FOUND" << std::endl;
          return;
        }

        tree = &treeIt->second;
      }

      auto endSections = tree->leafNodes( );
      if( endSections.empty( ))
       std::cout << "ERROR: " << gid << " leaf nodes ARE EMPTY!" << std::endl;

      for( auto node : endSections )
      {
       auto section = node->section( );

       auto it = _infoSections.find( section );
       if( it == _infoSections.end( ))
       {
         std::cout << "Section " << section->id( ) << " not parsed correctly." << std::endl;
         continue;
       }

       tSectionInfo& data = it->second;

       float maxDistance = 0;


       auto sectionSynapses = std::get< tsi_Synapses >( data );

       if( sectionSynapses.empty( ))
         continue;

       // Calculate farthest synapse distance on the section
       auto syn = sectionSynapses.begin( );
       auto farthestSynapse = syn;
       for( ; syn != sectionSynapses.end( ); ++syn )
       {
         float synapseDistanceOnSection =
             std::get< tpi_distance >( syn->second );

         if( synapseDistanceOnSection > maxDistance )
         {
           maxDistance = synapseDistanceOnSection;
           farthestSynapse = syn;
         }
       }

       tFixedSynapseInfo synapseInfo = farthestSynapse->second;

       vec3 synapsePos = std::get< tpi_position >( synapseInfo );
       unsigned int index = std::get< tpi_index >( synapseInfo );

       std::vector< vec3 > fixedSection =
           _cutEndSection( std::get< tsi_Interpolator >( data ).positions( ),
                          synapsePos, index );

       std::get< tsi_fixedSection >( data ) = fixedSection;
       std::get< tsi_leafSection >( data ) = true;

      } // for each end section
    } // for each gid
  }

  void PathFinder::addPostsynapticPath( nsol::MorphologySynapsePtr synapse,
                                        const tPosVec& nodes )
  {
    utils::PolylineInterpolation interpolator( nodes );

    _pathsPost.insert( std::make_pair( synapse, interpolator ));
  }



  cnode_ptr PathFinder::node( unsigned int sectionID ) const
  {
    return _treePre.node( sectionID );
  }

  utils::EventPolylineInterpolation PathFinder::computeDeepestPathFrom( unsigned int sectionID )
  {
    utils::EventPolylineInterpolation interpolator;

    auto origin = node( sectionID );
    if( !origin )
    {
      std::cout << "Error: node for section " << sectionID << " not found." << std::endl;

      return interpolator;
    }

    auto found = _computedPaths.find( sectionID );

    if( found != _computedPaths.end( ))
      return found->second;
    else
    {

      std::vector< ConnectivityNode* > pathSections;
      pathSections.reserve( origin->childrenMaxDepth( ) + 1);
      pathSections.push_back( origin );

      auto firstPath = origin->deepestPath( );

      for( auto pathSection : firstPath )
      {
        pathSections.push_back( pathSection );
      }

//      std::cout << "Found deepest path with " << pathSections.size( )
//                << " out of " << origin->childrenMaxDepth( )
//                << std::endl;

      auto transform = getTransform( _presynapticGID );

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

//        std::cout << "--------------------------------" << std::endl;

        //TODO Check if leaf section and add cut end
        auto infoSection = _infoSections.find( currentSection->section( ));
        if( infoSection != _infoSections.end( ) && std::get< tsi_leafSection >( infoSection->second ))
        {
          auto fixedSection = std::get< tsi_fixedSection >( infoSection->second );

          interpolator.insert( fixedSection );

//          std::cout << "Fixed ";
//          for( auto node : fixedSection )
//            std::cout << node << std::endl;
        }
        else
        {
          for( auto node : currentSection->section( )->nodes( ))
          {
            auto point = transformPoint( node->point( ), transform );

//            std::cout << point << std::endl;

            interpolator.insert( point );
          }

          if( last && last->numberOfChildren( ) > 1 )
          {
  //          auto children = last->children( );
    //          for( auto child = children.begin( ) + 1; child != children.end( ); ++child )
            {
              interpolator.addEventNode( prevDist, last->section( )->id() );
            }
          }
        }

        if( infoSection != _infoSections.end( ))
        {
          const auto& sectionSynapses = std::get< tsi_Synapses >( infoSection->second );
          for( auto synapse : sectionSynapses )
          {
            auto it = _synapseFixInfo->find( synapse.first );
            if( it != _synapseFixInfo->end( ))
            {
              auto presynInfo = std::get< TBSI_PRESYNAPTIC >( it->second );

              float synapseDist =
                  std::get< TBS_SEGMENT_DISTANCE >( presynInfo );


              unsigned int segmentIndex = std::get< TBS_SEGMENT_INDEX >( presynInfo );
              const auto& sectionInterp =
                  std::get< tsi_Interpolator >( infoSection->second );

              float sectionDist = sectionInterp.distance( segmentIndex );

              uintptr_t intPointer = reinterpret_cast< uintptr_t >( synapse.first );

              std::cout << "Adding synapse event " << synapse.first->gid( )
                        << " segment dist " << synapseDist
                        << " section dist " << sectionDist
                        << " current dist " << prevDist
                        << std::endl;

              synapseDist += prevDist;
              synapseDist += sectionDist;

              if( _pathsPost.find( synapse.first ) == _pathsPost.end( ) &&
                  synapse.first->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC )
              {
                auto postSections = pathToSoma( synapse.first, POSTSYNAPTIC );
                // TODO cut leaf section (first)

                auto postTransform = getTransform( synapse.first->postSynapticNeuron( ));

                tPosVec points;

                auto fixedSection = _infoSections.find( postSections.front( ));
                if( fixedSection == _infoSections.end( ))
                {
                  std::cout << "ERROR: Postsynaptic section " << postSections.front( )->id( )
                            << " info not found." << std::endl;
                  continue;
                }

                auto sectionEndNodes =
                    std::get< tsi_fixedSection >( fixedSection->second );

                for( auto nodeIt = sectionEndNodes.rbegin( );
                    nodeIt != sectionEndNodes.rend( ); ++nodeIt )
                  points.emplace_back( *nodeIt );

                for( unsigned int i = 1; i < postSections.size( ); ++i )
//                for( auto sec : postSections )
                {
                  auto sec = postSections[ i ];

                  for( auto nodeit = sec->nodes( ).rbegin( );
                      nodeit != sec->nodes( ).rend( ); ++nodeit )
//                    for( auto nodeit = sec->nodes( ).begin( ); nodeit != sec->nodes( ).end( ); ++nodeit )
                    points.emplace_back( transformPoint( (*nodeit)->point( ), postTransform ));
                }

                _pathsPost.insert( std::make_pair( synapse.first, points ));

                std::cout << "Added post path " << synapse.first
                          << " with " << points.size( ) << std::endl;

              }

              if( synapseDist > interpolator.totalDistance( ))
              {
                std::cout << "--Fixing synapse dist " << synapseDist
                          << " to " << interpolator.totalDistance( )
                          << std::endl;

                synapseDist = interpolator.totalDistance( );
              }
              interpolator.addEventNode( synapseDist, intPointer, utils::TEvent_synapse );
            }
          }
        }

        currentSection->sectionLength( interpolator.totalDistance( ) - prevDist );

        last = currentSection;
      }

//      std::cout << "Interpolator" << std::endl;
//      for( unsigned int i = 0; i < interpolator.size( ); ++i )
//        std::cout << interpolator[ i ]
//                  << "\t" << interpolator.distance( i )
//                  << "\t" << interpolator.direction( i ) << std::endl;


      auto it = _computedPaths.insert( std::make_pair( origin->section( )->id( ),
                                                       interpolator ));



      return it.first->second;
    }
  }


  utils::EventPolylineInterpolation PathFinder::getPostsynapticPath( nsolMSynapse_ptr synapse ) const
  {
    auto it = _pathsPost.find( synapse );
    if( it == _pathsPost.end( ))
    {
//      std::cout << "Error: post path for " << synapse << " NOT FOUND" << std::endl;
      return utils::EventPolylineInterpolation( );
    }

    return it->second;
  }

  std::vector< vec3 > PathFinder::cutLeafSection( unsigned int /*sectionID*/ ) const
  {
    tPosVec result;

    return result;
  }

}


