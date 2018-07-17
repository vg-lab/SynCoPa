/*
 * @file	PathFinder.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "PathFinder.h"

#include "PolylineInterpolation.hpp"

#include <unordered_set>

namespace syncopa
{

  static vec3 transformPoint( const vec3& point, const mat4& matrix )
  {
    vec4 transPoint( point.x( ), point.y( ), point.z( ), 1 );
    transPoint = matrix * transPoint;

    return transPoint.block< 3, 1 >( 0, 0 );
  }

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
//      calculateFixedSynapseSections( );
    }
  }


  bool sphereRayIntersect( vec3 center, float radius, vec3 start,
                           vec3 end, vec3& intersection )
  {
    vec3 result;

    float t0;
    float t1;

    float radius2 = radius * radius;

    vec3 l = center - start;
    vec3 dir = ( end - start ).normalized( );

    float tca = l.dot( dir );

    float d2 = l.dot(l) - tca * tca;

    if( d2 > radius2 )
      return false;

    float thc = sqrt(radius2 - d2);

    t0 = tca - thc;
    t1 = tca + thc;

    if (t0 > t1) std::swap(t0, t1);

    if (t0 < 0)
    {
      t0 = t1; // if t0 is negative, let's use t1 instead
      if (t0 < 0)
        return false; // both t0 and t1 are negative
    }

    intersection = dir * t0;

    return true;
  }


  std::vector< vec3 > PathFinder::getAllPathsPoints( unsigned int gid,
                                                     const std::set< unsigned int >& gidsPost,
                                                     float pointSize,
                                                     TNeuronConnection type ) const
  {
    nsol::Circuit::TDataType nsolType;

    unsigned int currentGid = gid;
    vec3 synapsePos;

//    if( type == PRESYNAPTIC )
    {
      nsolType = nsol::Circuit::PRESYNAPTICCONNECTIONS;
    }
//    else
//    {
//      nsolType = nsol::Circuit::POSTSYNAPTICCONNECTIONS;
//    }

    auto synapses =
          _dataset->circuit( ).synapses( gid, nsolType );

    std::cout << "Found " << synapses.size( ) << " synapses for " << gid << std::endl;



    auto endSections = findEndSections( synapses, type );

    auto synapseSections = parseSections( synapses, endSections, type );

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

        auto sectionNodes = findPathToSoma( msyn, type );

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

            std::cout << "Fixed section for synapse " << syn->preSynapticNeuron( ) << "-" << syn->postSynapticNeuron( )
                      << " with " << points.size( ) << " nodes of " << nodes.size( ) <<  std::endl;

          }
          else
          {
//            pathPoints.insert( 0, transformPoint( nodes.front( )->point( ), transform), vec3( 0, 0, 0 ));
//
//            for( unsigned int i = 0; i < nodes.size( ) - 1; ++i )
//            {
//              currentPoint = transformPoint( nodes[ i ]->point( ), transform );
//              nextPoint = transformPoint( nodes[ i + 1 ]->point( ), transform );
//
//              vec3 dir = nextPoint - currentPoint;
//              float module = dir.norm( );
//              accDist += module;
//              dir = dir / module;
//
//              pathPoints.insert( accDist, nextPoint, dir );
//            }
            for( auto node : nodes )
              pathPoints.insert( transformPoint( node->point( ), transform ));

//            accDist = pathPoints.totalDistance( );
          }

          float currentDist = 0;

          for( unsigned int i = 0; i < pathPoints.size( ); ++i )
//          while( currentDist < accDist )
          {
//            vec3 pos = pathPoints.pointAtDistance( currentDist );

            vec3 pos = pathPoints[ i ];
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
  PathFinder::findPathToSoma( const nsol::MorphologySynapsePtr synapse,
                              syncopa::TNeuronConnection type ) const
  {

    nsol::Nodes result;

//    unsigned int gid;
    nsol::NeuronMorphologySectionPtr section = nullptr;

    std::vector< nsol::NeuronMorphologySection* > sections;

    if( type == PRESYNAPTIC )
    {
//      gid = synapse->preSynapticNeuron( );
      section =
          dynamic_cast< nsol::NeuronMorphologySection* >(
              synapse->preSynapticSection( ));
    }
    else
    {
//      gid = synapse->postSynapticNeuron( );
      section =
          dynamic_cast< nsol::NeuronMorphologySection* >(
              synapse->postSynapticSection( ));
    }

//    auto morpho = _neuronMorphologies.find( gid );
//    if( !morpho )
//    {
//      std::cerr << "Morphology " << gid << " not found." << std::endl;
//      return result;
//    }

    while( section )
    {
      sections.push_back( section );

      section = dynamic_cast< nsol::NeuronMorphologySection* >( section->parent( ));
    }

//    for( auto sec : sections )
//    {
//      for( auto node : sec->nodes( ))
//        result.push_back( node );
//    }
//
//    return result;

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

  std::unordered_set< nsol::NeuronMorphologySectionPtr >
      PathFinder::findEndSections( const std::set< nsol::SynapsePtr >& synapses,
                                   TNeuronConnection type ) const
 {
    std::unordered_set< nsol::NeuronMorphologySectionPtr > candidates;
    std::unordered_set< nsol::NeuronMorphologySectionPtr > intermediate;

    for( auto syn : synapses )
    {
      auto msyn = dynamic_cast< nsol::MorphologySynapsePtr >( syn );
      if( !msyn )
        continue;

      nsol::NeuronMorphologySectionPtr sectionPtr =
          ( type == PRESYNAPTIC ? msyn->preSynapticSection( )
                                : msyn->postSynapticSection( ));

      if( !sectionPtr )
        continue;

      auto discarded = intermediate.find( sectionPtr );

      if( discarded == intermediate.end( ))
        candidates.insert( sectionPtr );

      sectionPtr = dynamic_cast< nsol::NeuronMorphologySectionPtr >( sectionPtr->parent( ));

      while( sectionPtr )
      {
        intermediate.insert( sectionPtr );
        candidates.erase( sectionPtr );

        sectionPtr = dynamic_cast< nsol::NeuronMorphologySectionPtr >( sectionPtr->parent( ));
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
  PathFinder::parseSections( const std::set< nsol::SynapsePtr >& synapses,
                             const tSectionsMap& endSections,
                             TNeuronConnection type ) const
  {

    tSectionsInfoMap result;
// TODO probar a pintar solo las secciones hoja
    for( auto syn : synapses )
    {

      auto msyn = dynamic_cast< nsol::MorphologySynapsePtr >( syn );

      if( msyn->synapseType( ) != nsol::MorphologySynapse::TSynapseType::AXODENDRITIC )
        continue;

      auto section =
          type == PRESYNAPTIC ? msyn->preSynapticSection( ) : msyn->postSynapticSection( );

      auto synapsePos =
          type == PRESYNAPTIC ? msyn->preSynapticSurfacePosition( ) : msyn->postSynapticSurfacePosition( );

      unsigned int currentGid = type == PRESYNAPTIC ? msyn->preSynapticNeuron( ) : msyn->postSynapticNeuron( );

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
            std::make_pair( msyn, projected ));

//          std::make_tuple( std::get< tpi_position >( projected ),
//                                                   std::get< tpi_distance >( projected ),
//                                                   std::get< tpi_distanceToSynapse >( projected ),
//                                                   std::get< tpi_index >( projected ))));
      }
      else
        std::cout << "Synapse " << msyn->preSynapticNeuron( )
                  << "-" << msyn->postSynapticNeuron( )
                  << " ... " << synapsePos.x( )
                  << ", " << synapsePos.y( )
                  << ", " << synapsePos.z( )
                  << " projection not found."
                  << std::endl;


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

  std::vector< nsol::NeuronMorphologySectionPtr >
  PathFinder::getChildrenSections( nsol::MorphologySynapsePtr synapse,
                                   TNeuronConnection type ) const
  {
    std::vector< nsol::NeuronMorphologySectionPtr > result;

    nsol::NeuronMorphologySectionPtr section =
        type == PRESYNAPTIC ? synapse->preSynapticSection( ) :
                              synapse->postSynapticSection( );

    if( !section )
      return result;

    std::queue< nsol::NeuronMorphologySectionPtr > pending;
    for( auto child: section->forwardNeighbors( ))
      pending.push( dynamic_cast< nsol::NeuronMorphologySectionPtr >( child ));

    while( pending.size( ) > 0 )
    {
      section = pending.front( );
      pending.pop( );

      result.push_back( section );

      for( auto child : section->forwardNeighbors( ))
      {
        pending.push( dynamic_cast< nsol::NeuronMorphologySectionPtr >( child ));
      }
    }

    return result;
  }

  void PathFinder::calculateFixedSynapseSections( void )
  {
    std::set< unsigned int > gids;
    for( auto neuron : _dataset->neurons( ))
    {
      gids.insert( neuron.first );
    }

    auto& circuit = _dataset->circuit(  );

    auto synapses = circuit.synapses( gids, nsol::Circuit::PRESYNAPTICCONNECTIONS );

    std::vector< nsol::MorphologySynapsePtr > synapseVec;
    synapseVec.reserve( synapses.size( ));
    for( auto syn : synapses )
      synapseVec.push_back( dynamic_cast< nsol::MorphologySynapsePtr >( syn ));

    unsigned int fixedSynapses = 0;

    for( auto syn : synapseVec )
    {
      unsigned int gidPre = syn->preSynapticNeuron( );
      unsigned int gidPost = syn->postSynapticNeuron( );

      nsol::NeuronMorphologySectionPtr section =
          syn->preSynapticSection( );

      vec3 synPos = syn->preSynapticSurfacePosition( );


      auto neuronIt = _fixedSynapseSections.find( gidPre );
      if( neuronIt == _fixedSynapseSections.end( ))
      {
        neuronIt = _fixedSynapseSections.insert(
            std::make_pair( gidPre, tSynapseFixedSections( ))).first;

      }

      tSynapseFixedSections& fixedSections = neuronIt->second;

      auto prevSections = findPathToSoma( syn, PRESYNAPTIC );
      auto postSections = getChildrenSections( syn, PRESYNAPTIC );

      std::vector< nsol::NeuronMorphologySectionPtr > sections;
      sections.insert( sections.end( ), prevSections.begin( ), prevSections.end( ));
      sections.insert( sections.end( ), postSections.begin( ), postSections.end( ));

      unsigned int minDistIdx = sections.size( );
      float minDist = std::numeric_limits< float >::max( );

      auto transform = getTransform( gidPre );

//      std::vector< float > distances;
//      std::vector< vec3 > positions;
//      for( auto sec : sections )
//      {
//        for( auto node : sec->nodes( ))
//        {
//          positions.push_back( transformPoint( node->point( ), transform ));
//        }
//      }
//
//      distances.reserve( positions.size( ));
//      for( unsigned int i = 0; i < positions.size( ); ++i )
//      {
//        vec3 pos = positions[ i ];
//
//        float dist = ( synPos - pos ).norm( );
//
//        if( dist < minDist )
//        {
//          minDist = dist;
//          minDistIdx = i;
//        }
//
//        distances.push_back( dist);
//
//      }
//
//      std::cout << "Min distance " << minDist << std::endl;




      std::vector< tFixedSynapseInfo > projectedSynapses( sections.size( ));

      unsigned int defSectionIdx = sections.size( );
//      #pragma omp parallel for
      for( int i = 0; i < ( int ) sections.size( ); ++i )
      {
        nsol::NeuronMorphologySectionPtr sec = sections[ i ];

        utils::PolylineInterpolation interpolator;
        for( auto node : sec->nodes( ))
          interpolator.insert( transformPoint( node->point( ), transform ));

        auto projected = findClosestPointToSynapse( synPos, interpolator );
        float currentDist = std::get< tpi_distanceToSynapse >( projected );

//        std::cout << "Current dist " << currentDist << std::endl;

        if( sec->id( ) == section->id( ))
          defSectionIdx = i;

        if( currentDist <  minDist )
        {
          minDist = currentDist;
          minDistIdx = i;

        }

        projectedSynapses[ i ] = projected;
      }

      if( minDistIdx < sections.size( ))
      {


        if( section != sections[ minDistIdx])
        {
          std::cout << "Projected synapse " << gidPre << "-" << gidPost
                  << " into closest section " << sections[ minDistIdx]
                  << " " << minDistIdx
                  << " dist " << minDist
                  << " instead of " << section
                  << " dist " << std::get< tpi_distanceToSynapse >( projectedSynapses[ defSectionIdx ])
                  << std::endl;

          ++fixedSynapses;
        }
        fixedSections.insert(
            std::make_pair( syn, std::make_pair( sections[ minDistIdx ],
                                                 sections[ minDistIdx ] )));
      }
    }

    std::cout << "Fixed synapses " << fixedSynapses << " out of " << synapses.size( ) << std::endl;

  }


}


