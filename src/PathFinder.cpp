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

  PathFinder::PathFinder( nsol::DataSet* dataset_ )
  {
    dataset( dataset_ );
  }

  PathFinder::~PathFinder( void )
  {

  }

  void PathFinder::dataset( nsol::DataSet* dataset_ )
  {
    _dataset = dataset_;

    if( _dataset )
    {
      brion::BlueConfig* blueConfig = _dataset->blueConfig( );

      brain::Circuit brainCircuit( *blueConfig);
      brion::GIDSet gidSetBrain = brainCircuit.getGIDs( );

      const brain::Synapses& brainSynapses =
          brainCircuit.getAfferentSynapses( gidSetBrain,
                                            brain::SynapsePrefetch::all );

      auto synapses = _dataset->circuit( ).synapses( );
      for( unsigned int i = 0; i < synapses.size( ); ++i )
      {
        auto synapse = dynamic_cast< nsolMSynapse_ptr >( synapses[ i ]);
        auto brainSynapse = brainSynapses[ i ];

        tBrainSynapse infoPre =
            std::make_tuple( synapse,
                             brainSynapse.getPresynapticSectionID( ),
                             brainSynapse.getPresynapticSegmentID( ),
                             brainSynapse.getPresynapticDistance( ));

        tBrainSynapse infoPost =
            std::make_tuple( synapse,
                             brainSynapse.getPostsynapticSectionID( ),
                             brainSynapse.getPostsynapticSegmentID( ),
                             brainSynapse.getPostsynapticDistance( ));


        _synapseFixInfo.insert(
            std::make_pair( synapse, std::make_pair( infoPre, infoPost )));

      }

      std::cout << "Loaded BRAIN synapse info of " << _synapseFixInfo.size( ) << std::endl;
      //      calculateFixedSynapseSections( );
    }
  }

  std::vector< vec3 > PathFinder::getAllPathsPoints( unsigned int gid,
                                                     const std::set< unsigned int >& gidsPost,
                                                     float pointSize,
                                                     TNeuronConnection type ) const
  {

    unsigned int currentGid = gid;
    vec3 synapsePos;

    std::set< unsigned int > gids = { gid };
    auto& synapses = getSynapses( );

    std::cout << "Found " << synapses.size( ) << " synapses for " << gid << std::endl;

    auto synapseSections = processSections( synapses, type, currentGid );
//    auto synapseSections = parseSections( synapses, type );

    std::unordered_set< nsol::NeuronMorphologySectionPtr > insertedSections;

    std::vector< vec3 > sectionPoints;

    std::vector< mat4 > transforms;
    std::vector< nsol::Nodes > presynapticPaths;
    std::vector< nsol::Nodes > postsynapticPaths;

    std::vector< vec3 > result;

    for( auto syn : synapses )
    {
      if( gidsPost.empty( ) || gidsPost.find( syn->postSynapticNeuron( )) != gidsPost.end( ))
      {
        auto msyn = dynamic_cast< nsol::MorphologySynapsePtr >( syn );
        if( !msyn )
        {
          std::cerr << "Error converting from Synapse to MorphologySynapse " << syn << std::endl;
          continue;
        }

        if( type == POSTSYNAPTIC )
        {
          currentGid = syn->postSynapticNeuron( );
          synapsePos = msyn->postSynapticSurfacePosition( );
        }
        else
          synapsePos = msyn->preSynapticSurfacePosition( );


        auto transform = getTransform( currentGid );

        auto sectionNodes = pathToSoma( msyn, type );

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

//          float accDist = 0;

          vec3 currentPoint;
          vec3 nextPoint;

          utils::PolylineInterpolation pathPoints;

          auto parsedSection = synapseSections.find( section );
          if( parsedSection != synapseSections.end( ) &&
              std::get< tsi_leafSection >( parsedSection->second ))
          {
            auto points = std::get< tsi_fixedSection >( parsedSection->second );

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
      }
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


  std::set< unsigned int > PathFinder::connectedTo( unsigned int gid ) const
  {
    std::set< unsigned int > result;

    auto synapses = _dataset->circuit( ).synapses( gid, nsol::Circuit::PRESYNAPTICCONNECTIONS );

    for( auto syn : synapses )
    {
      result.insert( syn->postSynapticNeuron( ));
    }

    return result;
  }

  unsigned int PathFinder::findSynapseSegment(  const vec3& synapsePos,
                                                const nsol::Nodes& nodes ) const
  {
    unsigned int index = nodes.size( );

    vec3 currentPoint;
    vec3 nextPoint;

//    std::cout << "Syn:  " << synapsePos.x( )
//              << ", " << synapsePos.y( )
//              << ", " << synapsePos.z( )
//              << std::endl;


    for( unsigned int i = 0; i < nodes.size( ) - 1; ++i )
    {
      currentPoint = nodes[ i ]->point( );
      nextPoint = nodes[ i + 1 ]->point( );


//      std::cout << "Node: " << currentPoint.x( )
//                << ", " << currentPoint.y( )
//                << ", " << currentPoint.z( )
//                << std::endl;


      vec3 ab = nextPoint - currentPoint;
      vec3 ac = synapsePos - currentPoint;

//      if( ab.cross( ac ).norm( ) != 0.0f )
//        continue;

      float kAC = ab.dot( ac );
      float kAB = ab.dot( ab );

      if( kAC == 0 || ( kAC > 0 && kAC < kAB ))
      {
        index = i;
        break;
      }

    }

    return index;
  }

  std::unordered_set< nsolMSection_ptr>
      PathFinder::findEndSections( const std::vector< nsolMSynapse_ptr >& synapses,
                                   TNeuronConnection type ) const
 {
    std::unordered_set< nsol::NeuronMorphologySectionPtr > candidates;
    std::unordered_set< nsol::NeuronMorphologySectionPtr > intermediate;

    for( auto syn : synapses )
    {
      nsolMSection_ptr sectionPtr =
          ( type == PRESYNAPTIC ? syn->preSynapticSection( ) : syn->postSynapticSection( ));

      if( !sectionPtr )
        continue;

      auto discarded = intermediate.find( sectionPtr );

      if( discarded == intermediate.end( ))
        candidates.insert( sectionPtr );

      sectionPtr = dynamic_cast< nsolMSection_ptr >( sectionPtr->parent( ));

      while( sectionPtr )
      {
        intermediate.insert( sectionPtr );
        candidates.erase( sectionPtr );

        sectionPtr = dynamic_cast< nsolMSection_ptr >( sectionPtr->parent( ));
      }

    }
    return candidates;
 }

  std::vector< vec3 > PathFinder::cutEndSection( const std::vector< vec3 >& nodes,
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

  tSectionsInfoMap
  PathFinder::processSections( const std::vector< nsolMSynapse_ptr >& synapses,
                               TNeuronConnection type, unsigned int neuronGID ) const
  {
    tSectionsInfoMap result;

    const ConnectivityTree* tree = nullptr;

    if( type == PRESYNAPTIC )
    {
      tree = &_treePre;

    }
    else
    {
      auto treeIt = _treePost.find( neuronGID );
      if( treeIt == _treePost.end( ))
        return result;

      tree = &treeIt->second;
    }

    auto transform = getTransform( neuronGID );

    for( auto synapse : synapses )
    {
      auto synInfo = _synapseFixInfo.find( synapse );
      if( synInfo == _synapseFixInfo.end( ))
      {
        std::cout << "ERROR: Synapse " << synapse << " info NOT FOUND" << std::endl;
        continue;
      }

      const auto& synapseFixInfo = ( type == PRESYNAPTIC )
                                   ? synInfo->second.first
                                   : synInfo->second.second;

      nsolMSection_ptr section =
          ( type == PRESYNAPTIC ) ? synapse->preSynapticSection( )
                                  : synapse->postSynapticSection( );

      if( !section )
      {
        std::cout << "ERROR: Synapse " << synapse << " section NOT FOUND" << std::endl;
        continue;
      }

      if( std::get< tsfi_section >( synapseFixInfo ) != section->id( ))
      {
        std::cout << "ERROR: Section " << section->id( )
                  << " does not match " << std::get< tsfi_section >( synapseFixInfo )
                  << std::endl;
        continue;
      }

      auto it = result.find( section );
      if( it == result.end( ))
      {
        utils::PolylineInterpolation interpolator;

        for( auto node : section->nodes( ))
        {
          vec4 transPoint( node->point( ).x( ), node->point( ).y( ),
                           node->point( ).z( ), 1 );

          transPoint = transform * transPoint;

          interpolator.insert( transPoint.block< 3, 1>( 0, 0 ));
        }

        std::unordered_map< nsol::MorphologySynapsePtr,
                            tFixedSynapseInfo> synapseSet;

        float sectionDist = interpolator.totalDistance( );

        std::vector< vec3 > fixedSection;

        tSectionInfo data =
            std::make_tuple( synapseSet, sectionDist,
                             false,
                             fixedSection, interpolator );

        it = result.insert( std::make_pair( section, data )).first;

      } // if section not found

      auto& synapseMap = std::get< tsi_Synapses >( it->second );

      unsigned int segmentIndex = std::get< tsfi_node >( synapseFixInfo );
      const auto& nodes = section->nodes( );

      if( nodes.empty( ) || nodes.size( ) == 1)
      {
        std::cout << "ERROR: Section " << section->id( )
                  << " with " << nodes.size( )
                  << " nodes." << std::endl;
        continue;
      }

      if( segmentIndex > nodes.size( ) - 1)
      {
        std::cout << "ERROR: " << neuronGID << " Node index " << segmentIndex
                  << " is greater than section " << section->id( )
                  << " nodes number " << nodes.size( )
                  << " distance " << std::get< tsfi_distance >( synapseFixInfo )
                  << std::endl;
        continue;
      }

      float distance = std::get< tsfi_distance >( synapseFixInfo );
      vec3 start = nodes[ segmentIndex ]->point( );
      vec3 end = nodes[ segmentIndex + 1 ]->point( );

      float length = ( end - start ).norm( );

      float normalized = distance / length;

      vec3 synapsePos = transformPoint( end * normalized + start * ( 1 - normalized ), transform );
//

      tFixedSynapseInfo fixedSynInfo = std::make_tuple( synapsePos, distance, 0, segmentIndex, true );

      synapseMap.insert( std::make_pair( synapse, fixedSynInfo ));

    } // for each synapse


    auto endSections = tree->leafNodes( );

    for( auto node : endSections )
    {
      auto section = node->section( );

      auto it = result.find( section );
        if( it == result.end( ))
          std::cout << "Section " << section->id( ) << " not parsed correctly." << std::endl;
        else
        {
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

          vec3 synapsePos = std::get< tsy_fixedPosition >( synapseInfo );
          unsigned int index = std::get< tsy_indexLastNode >( synapseInfo );

          std::cout << std::endl << " selected " << maxDistance << std::endl;

          std::vector< vec3 > fixedSection =
              cutEndSection( std::get< tsi_Interpolator >( data ).positions( ),
                             synapsePos, index );

          std::get< tsi_fixedSection >( data ) = fixedSection;
          std::get< tsi_leafSection >( data ) = true;

        }

    }

    return result;
  }

  tSectionsInfoMap
  PathFinder::parseSections( const std::vector< nsolMSynapse_ptr >& synapses,
                             TNeuronConnection type ) const
  {

    auto treeEndSections = _treePre.leafNodes( );
    auto endSections = findEndSections( synapses, type );

//    if( treeEndSections.size( ) != endSections.size( ))
//      while(true)
//      {
//        std::cout << "ERRRRRRROR " << treeEndSections.size( ) << " " << endSections.size( ) << std::endl;
//        std::cout << "Tree ";
//        for( auto node : treeEndSections )
//          std::cout << " " << node->section( )->id( );
//
//        std::cout << std::endl << "Other ";
//        for( auto section : endSections )
//          std::cout << " " << section->id( );
//        std::cout << std::endl;
//      }


    tSectionsInfoMap result;
// TODO probar a pintar solo las secciones hoja
    for( auto syn : synapses )
    {
      if( !syn || syn == nullptr )
      {
        std::cout << "ERROR EMPTY SYNAPSE " << std::endl;
        continue;
      }
//      auto msyn = dynamic_cast< nsol::MorphologySynapsePtr >( syn );

//      if( msyn->synapseType( ) != nsol::MorphologySynapse::TSynapseType::AXODENDRITIC )
//        continue;

      auto section =
          type == PRESYNAPTIC ? syn->preSynapticSection( ) : syn->postSynapticSection( );

      if( !section )
      {
        std::cout << "Avoiding somatic synapse " << syn->preSynapticNeuron( )
                  << "-" << syn->postSynapticNeuron( ) << std::endl;
        continue;
      }

      auto synapsePos =
          type == PRESYNAPTIC ? syn->preSynapticSurfacePosition( ) : syn->postSynapticSurfacePosition( );

      unsigned int currentGid = type == PRESYNAPTIC ? syn->preSynapticNeuron( ) : syn->postSynapticNeuron( );

      auto transform = getTransform( currentGid );

//      std::cout << "Synapse " << msyn->preSynapticNeuron( )
//                << "-" << msyn->postSynapticNeuron( )
//                << " ... " << synapsePos.x( )
//                << ", " << synapsePos.y( )
//                << ", " << synapsePos.z( )
//                << std::endl;
//
//      std::cout << "--------------------------------------------" << std::endl;


      auto it = result.find( section );

      // If section has been already processed, add new synapse
      if( it == result.end( ))
      {
        utils::PolylineInterpolation interpolator;

        for( auto node : section->nodes( ))
        {
          vec4 transPoint( node->point( ).x( ), node->point( ).y( ), node->point( ).z( ), 1 );
          transPoint = transform * transPoint;

          interpolator.insert( transPoint.block< 3, 1>( 0, 0 ));
        }

        std::unordered_map< nsol::MorphologySynapsePtr,
                            tFixedSynapseInfo> synapseSet;

        float sectionDist = interpolator.totalDistance( );

        std::vector< vec3 > fixedSection;

        tSectionInfo data =
            std::make_tuple( synapseSet, sectionDist,
                             false,
                             fixedSection, interpolator );

        it = result.insert( std::make_pair( section, data )).first;
      }

      auto projected =
          findClosestPointToSynapse( synapsePos,
                          std::get< tsi_Interpolator >( it->second ));

      if( std::get< tpi_found >( projected ))
      {
        std::get< tsi_Synapses >( it->second ).insert(
            std::make_pair( syn, projected ));

//          std::make_tuple( std::get< tpi_position >( projected ),
//                                                   std::get< tpi_distance >( projected ),
//                                                   std::get< tpi_distanceToSynapse >( projected ),
//                                                   std::get< tpi_index >( projected ))));
      }
//      else
//        std::cout << "Synapse " << syn->preSynapticNeuron( )
//                  << "-" << syn->postSynapticNeuron( )
//                  << " ... " << synapsePos.x( )
//                  << ", " << synapsePos.y( )
//                  << ", " << synapsePos.z( )
//                  << " projection not found."
//                  << std::endl;


    }

    // Store end sections information
    for( auto section : endSections )
    {
      auto it = result.find( section );
      if( it == result.end( ))
        std::cout << "Section " << section->id( ) << " not parsed correctly." << std::endl;
      else
      {
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
              std::get< tsy_distanceOnSection >( syn->second );

          if( synapseDistanceOnSection > maxDistance )
          {
            maxDistance = synapseDistanceOnSection;
            farthestSynapse = syn;
          }
        }

        tFixedSynapseInfo synapseInfo = farthestSynapse->second;

        vec3 synapsePos = std::get< tsy_fixedPosition >( synapseInfo );
        unsigned int index = std::get< tsy_indexLastNode >( synapseInfo );

        std::vector< vec3 > fixedSection =
            cutEndSection( std::get< tsi_Interpolator >( data ).positions( ),
                           synapsePos, index );


        std::get< tsi_fixedSection >( data ) = fixedSection;
        std::get< tsi_leafSection >( data ) = true;

      }
    }

    return result;

  }

  tFixedSynapseInfo PathFinder::projectSynapse( const vec3 synapsePos,
                                                const utils::PolylineInterpolation& nodes ) const
  {

    unsigned int index = nodes.size( );

    bool found = false;

    vec3 projectedPos;
    float projectedDist;
    float synapseDist;

    std::vector< vec3 > projectedPoints;
    std::vector< float > projectedDistances;
    std::vector< float > distanceToProjectedPoint;

    projectedPoints.reserve( nodes.size( ) - 1 );
    projectedDistances.reserve( nodes.size( ) - 1 );
    distanceToProjectedPoint.resize( nodes.size( ) - 1, std::numeric_limits< float >::max( ));

    vec3 currentPoint;
    vec3 nextPoint;

    vec3 ab;
    vec3 ac;

    float kAC;
//    float kAB;

//    std::cout << "Synapse " << synapsePos.x( )
//              << ", " << synapsePos.y( )
//              << ", " << synapsePos.z( )
//              << std::endl;
//
//    std::cout << "Node 0 " << nodes.firstPosition( ).x( )
//               << ", " << nodes.firstPosition( ).y( )
//               << ", " << nodes.firstPosition( ).z( )
//               << std::endl;

    for( unsigned int i = 0; i < nodes.size( ) - 1; ++i )
    {
      currentPoint = nodes[ i ];
      nextPoint = nodes[ i + 1 ];

//      ab = nextPoint - currentPoint;
      ab = nodes.segmentDirection( i );
      ac = synapsePos - currentPoint;

      kAC = ab.dot( ac );
//      kAB = ab.dot( ab );

      projectedPos = vec3( 0, 0, 0 );
      projectedDist = std::numeric_limits< float >::max( );
      synapseDist = std::numeric_limits< float >::max( );

//      std::cout << "Node " << i << " " << nextPoint.x( )
//                 << ", " << nextPoint.y( )
//                 << ", " << nextPoint.z( );

      if( kAC >= 0 )
      {

//        projectedDist = kAC / nodes.segmentDistance( i );
        projectedDist = kAC;
        projectedDist = std::min( nodes.segmentDistance( i ),
                                  std::max( projectedDist, 0.0f ));

        projectedPos =
            nodes[ i ] + nodes.segmentDirection( i ) * projectedDist;

        synapseDist = ( projectedPos - nodes[ i ]).norm( );


//        std::cout << " -> " << synapseDist
//                  << " " << projectedPos.x( )
//                  << ", " << projectedPos.y( )
//                  << ", " << projectedPos.z( );

        found = true;

      }

//      std::cout << std::endl;

      projectedPoints.push_back( projectedPos );
      distanceToProjectedPoint.push_back( synapseDist );
      projectedDistances[ i ] = projectedDist;


    }

    index = distanceToProjectedPoint.size( );
    float minDist = std::numeric_limits< float >::max( );
    for( unsigned int i = 0; i < distanceToProjectedPoint.size( ); ++i )
    {
      float currentDist = distanceToProjectedPoint[ i ];
      if( currentDist > 0 && currentDist < minDist )
      {
        index = i;
        minDist = currentDist;
      }
    }


    projectedPos = projectedPoints[ index ];
    projectedDist = projectedDistances[ index ];

//    std::cout << "Selected " << index
//              << " with " << distanceToProjectedPoint[ index ]
//              << " "<< projectedPos.x( )
//              << ", " << projectedPos.y( )
//              << ", " << projectedPos.z( )
//              << std::endl;
//
//
//      std::cout << "Initial pos " << synapsePos.x( )
//                                << ", " << synapsePos.y( )
//                                << ", " << synapsePos.z( )
//              << " Projected pos " << projectedPos.x( )
//                            << ", " << projectedPos.y( )
//                            << ", " << projectedPos.z( )
//                            << std::endl;

    float totalDist = nodes.distance( index ) + projectedDist;

    return std::make_tuple( projectedPos, totalDist,
                            distanceToProjectedPoint[ index ], index, found );

//      if( ab.cross( ac ).norm( ) != 0.0f )
//        continue;


  }

  tFixedSynapseInfo
    PathFinder::findClosestPointToSynapse( const vec3 synapsePos,
                                           const utils::PolylineInterpolation& nodes ) const
  {
    bool found = false;

    vec3 projectedPos;
    float projectedDist;
//    float synapseDist;

    vec3 startPoint;
//    vec3 endPoint;
    std::vector< float > distances;
    distances.reserve( nodes.size( ));

    float minDist = std::numeric_limits< float >::max( );
    unsigned int index = nodes.size( ) - 1;

//        std::cout << "-----------------------------" << std::endl
//                  << "Synapse " << synapsePos.x( )
//                  << ", " << synapsePos.y( )
//                  << ", " << synapsePos.z( )
//                  << std::endl;

    for( unsigned int i = 0; i < nodes.size( ); ++i )
    {
      vec3 currentPoint = nodes[ i ];
      float dist = ( synapsePos - currentPoint ).norm( );
      if( dist < minDist )
      {
        minDist = dist;
        index = i;
      }

//      std::cout << "Calculated " << i
//                    << " with " << dist
//                    << " -> "<< currentPoint.x( )
//                    << ", " << currentPoint.y( )
//                    << ", " << currentPoint.z( )
//                    << std::endl;

      distances.push_back( dist );
    }


    if( index == nodes.size( ) - 1 ||
        ( index > 0 && distances[ index - 1] < distances[ index + 1 ]))
    {
      --index;
    }

    startPoint = nodes[ index ];

    projectedDist =
        nodes.segmentDirection( index ).dot( ( synapsePos - startPoint ));
//
//    std::cout << "Projected synapse at node " << index
//              << " with distance " << projectedDist
//              << " out of " << nodes.segmentDistance( index )
//              << " -> " << distances[ index ]
//              << std::endl;

    if( projectedDist < 0 || projectedDist > nodes.segmentDistance( index ) * 2)
    {
//      found = false;
    }
    else
    {
      projectedDist =
          std::max( 0.f, std::min( projectedDist,
                                   nodes.segmentDistance( index )));
      found = true;
    }

    projectedPos = nodes[ index ] +
        nodes.segmentDirection( index ) * projectedDist;

    float totalDist = nodes.distance( index ) + projectedDist;

    return std::make_tuple( projectedPos, totalDist, distances[ index ],
                            index, found );


  }

  void PathFinder::addPostsynapticPath( nsol::MorphologySynapsePtr synapse,
                                        const tPosVec& nodes )
  {
    utils::PolylineInterpolation interpolator( nodes );

    _pathsPost.insert( std::make_pair( synapse, interpolator ));
  }


  void PathFinder::configure( unsigned int presynapticGid )
  {
    clear( );

    _loadSynapses( presynapticGid );

    auto& synapses =  getSynapses( );

    _infoSections = parseSections( synapses );


    _presynapticGID = presynapticGid;

    unsigned int maxDepth = 0;

    for( auto syn : synapses )
    {
      auto section =
          dynamic_cast< nsol::NeuronMorphologySectionPtr >( syn->preSynapticSection( ));

      if( !section )
      {
        std::cout << "ERROR: Unassigned synapse section " << syn
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

      if( !_treePre.hasNode( section ))
      {
        auto sections = pathToSoma( section );

        unsigned int depth = _treePre.addBranch( sections );

        if( depth > maxDepth )
          maxDepth = depth;
      }
    }

    _maxDepth = maxDepth;

    _treePre.print( );

  }

  const ConnectivityTree& PathFinder::tree( void ) const
  {
    return _treePre;
  }


  void PathFinder::clear( void )
  {

    _fixedSynapseSections.clear( );
    _computedPaths.clear( );
    _treePre.clear( );

    _pathsPost.clear( );

    _infoSections.clear( );

    _synapses.clear( );

    _maxDepth = 0;
  }

  cnode_ptr PathFinder::node( unsigned int sectionID ) const
  {
    return _treePre.node( sectionID );
  }

  utils::EventPolylineInterpolation PathFinder::computedPathFrom( unsigned int sectionID )
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

      for( auto node : firstPath )
      {
        pathSections.push_back( node );
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

//        std::cout << "-------------" << std::endl;

        //TODO Check if leaf section and add cut end
        auto infoSection = _infoSections.find( currentSection->section( ));
//        if( infoSection != _infoSections.end( ) && std::get< tsi_leafSection >( infoSection->second ))
//        {
//          interpolator.insert( std::get< tsi_fixedSection >( infoSection->second ));
//        }
//        else
        {
          for( auto node : currentSection->section( )->nodes( ))
          {
            auto point = transformPoint( node->point( ), transform );

//            std::cout << point << std::endl;

            interpolator.insert( point );
          }

  //        for( auto nodeIt = sec->nodes( ).rbegin( ); nodeIt != sec->nodes( ).rend( ); ++nodeIt )
  //          nodes.push_back( transformPoint( ( *nodeIt )->point( ), transform ));
          if( last && last->numberOfChildren( ) > 1 )
          {
  //          auto children = last->children( );
    //          for( auto child = children.begin( ) + 1; child != children.end( ); ++child )
            {
              interpolator.addEventNode( prevDist, last->section( )->id() );
            }
          }

          if( infoSection != _infoSections.end( ))
          {

          }

        }
        currentSection->sectionLength( interpolator.totalDistance( ) - prevDist );

        last = currentSection;
      }

      auto it = _computedPaths.insert( std::make_pair( origin->section( )->id( ), interpolator ));

      return it.first->second;
    }
  }

//  std::vector< nsolMSynapse_ptr > PathFinder::getSynapses( unsigned int gidPre ) const
//  {
//    std::set< unsigned int > gids = { gidPre };
//    return getSynapses( gids );
//  }


  const std::vector< nsolMSynapse_ptr >& PathFinder::getSynapses( void ) const
  {
    return _synapses;
  }


  void PathFinder::_loadSynapses( unsigned int presynapticGID )
  {
    if( presynapticGID == _presynapticGID )
      return;

    std::vector< nsolMSynapse_ptr > result;

    std::set< unsigned int > gidsPre = { presynapticGID };

    auto& circuit = _dataset->circuit( );
    auto synapseSet = circuit.synapses( gidsPre,
                                        nsol::Circuit::PRESYNAPTICCONNECTIONS );


    result.reserve( synapseSet.size( ));

    for( auto syn : synapseSet )
    {
      auto msyn = dynamic_cast< nsolMSynapse_ptr >( syn );
      if( !msyn || !msyn->preSynapticSection( ) || !msyn->postSynapticSection( ))
      {
        std::cout << "EMPTY SYNAPSE " << std::endl;
        continue;
      }

      result.push_back( msyn );
    }

    _synapses = result;

  }
//
//  std::vector< nsolMSynapse_ptr > PathFinder::getSynapses( const std::set< unsigned int >& gidsPre ) const
//  {
//
//
//    std::vector< nsolMSynapse_ptr > result;
//
//
//
//    return result;
//  }

}


