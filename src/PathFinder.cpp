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
  : _dataset( dataset_ )
  {

  }

  PathFinder::~PathFinder( void )
  {

  }

  void PathFinder::dataset( nsol::DataSet* dataset_ )
  {
    _dataset = dataset_;
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

    auto synapseSections = parseSections( synapses, type );

    std::unordered_set< nsol::NeuronMorphologySectionPtr > insertedSections;

    std::vector< vec3 > sectionPoints;

    std::vector< mat4 > transforms;
    std::vector< nsol::Nodes > presynapticPaths;
    std::vector< nsol::Nodes > postsynapticPaths;

    std::vector< vec3 > result;

    auto endSections = findEndSections( synapses, type );

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

          float accDist = 0;

          vec3 currentPoint;
          vec3 nextPoint;

          utils::PolylineInterpolation pathPoints;

          auto parsedSection = synapseSections.find( section );
          if( parsedSection != synapseSections.end( ) &&
              std::get< tsi_leafSection >( parsedSection->second ))
          {
            auto points = std::get< tsi_fixedSection >( parsedSection->second );

            pathPoints.insert( points );
            accDist = pathPoints.totalDistance( );

            std::cout << "Fixed section for synapse " << syn->preSynapticNeuron( ) << "-" << syn->postSynapticNeuron( )
                      << " with " << points.size( ) << " nodes of " << nodes.size( ) <<  std::endl;

          }
          else
          {
            pathPoints.insert( 0, transformPoint( nodes.front( )->point( ), transform), vec3( 0, 0, 0 ));

            for( unsigned int i = 0; i < nodes.size( ) - 1; ++i )
            {
              currentPoint = transformPoint( nodes[ i ]->point( ), transform );
              nextPoint = transformPoint( nodes[ i + 1 ]->point( ), transform );

              vec3 dir = nextPoint - currentPoint;
              float module = dir.norm( );
              accDist += module;
              dir = dir / module;

              pathPoints.insert( accDist, nextPoint, dir );
            }

          }

          float currentDist = 0;

          while( currentDist < accDist )
          {
            vec3 pos = pathPoints.pointAtDistance( currentDist );

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

    for( unsigned int i = 0; i < lastIndex; ++i )
    {
      result.push_back( nodes[ i ]);
    }

    result.push_back( synapsePos );

//    result = nodes;

    return result;
  }

  tSectionsInfoMap
  PathFinder::parseSections( const std::set< nsol::SynapsePtr >& synapses,
                             TNeuronConnection type ) const
  {

    tSectionsInfoMap result;

    auto endSections = findEndSections( synapses, type );

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

      std::cout << "Synapse " << msyn->preSynapticNeuron( )
                << "-" << msyn->postSynapticNeuron( )
                << " ... " << synapsePos.x( )
                << ", " << synapsePos.y( )
                << ", " << synapsePos.z( )
                << std::endl;

      std::cout << "--------------------------------------------" << std::endl;

      {
        auto it = result.find( section );
        if( it != result.end( ))
        {
          auto projected =
              projectSynapse( synapsePos,
                              std::get< tsi_Interpolator >( it->second ));

          if( std::get< tpi_found >( projected ))
          {
            std::get< tsi_Synapses >( it->second ).insert(
                std::make_pair( msyn, std::make_tuple( std::get< tpi_position >( projected ),
                                                       std::get< tpi_distance >( projected ),
                                                       std::get< tpi_index >( projected ))));
          }
          else
            std::cout << "Synapse " << msyn->preSynapticNeuron( )
                      << "-" << msyn->postSynapticNeuron( )
                      << " projection not found."
                      << std::endl;
        }
        else
        {
          utils::PolylineInterpolation interpolator;

          for( auto node : section->nodes( ))
          {
            vec4 transPoint( node->point( ).x( ), node->point( ).y( ), node->point( ).z( ), 1 );
            transPoint = transform * transPoint;

            interpolator.insert( transPoint.block< 3, 1>( 0, 0 ));
          }

          std::unordered_map< nsol::MorphologySynapsePtr,
                              std::tuple< vec3, float, unsigned int >> synapseSet;

          auto projected = projectSynapse( synapsePos, interpolator );
          if( std::get< tpi_found >( projected ))
          {
            synapseSet.insert( std::make_pair( msyn, std::make_tuple( std::get< tpi_position >( projected ),
                                                                      std::get< tpi_distance >( projected ),
                                                                      std::get< tpi_index >( projected ))));
          }
          else
            std::cout << "Synapse " << msyn->preSynapticNeuron( )
                      << "-" << msyn->postSynapticNeuron( )
                      << " projection not found."
                      << std::endl;

          float sectionDist = interpolator.totalDistance( );

          std::vector< vec3 > fixedSection;

          tSectionInfo data =
              std::make_tuple( synapseSet, sectionDist,
                               false,
                               fixedSection, interpolator );

          result.insert( std::make_pair( section, data ));

        }

//        section = dynamic_cast< nsol::NeuronMorphologySectionPtr >( section->parent( ));
      }

    }

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

        auto syn = sectionSynapses.begin( );
        auto furthestSynapse = syn;
        for( ; syn != sectionSynapses.end( ); ++syn )
        {
          float synapseDistanceOnSection =
              std::get< tsy_distanceOnSection >( syn->second );

          if( synapseDistanceOnSection > maxDistance )
          {
            maxDistance = synapseDistanceOnSection;
            furthestSynapse = syn;
          }
        }

        tFixedSynapseInfo synapseInfo = furthestSynapse->second;

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

  std::tuple< vec3, float, unsigned int, bool >
  PathFinder::projectSynapse( const vec3 synapsePos,
                              const utils::PolylineInterpolation& nodes ) const
  {

    unsigned int index = nodes.size( );
    float projectedDist = 0.0f;
    bool found = false;

    vec3 currentPoint;
    vec3 nextPoint;

    vec3 ab;
    vec3 ac;

    float kAC;
    float kAB;

    std::cout << "Synapse " << synapsePos.x( )
              << ", " << synapsePos.y( )
              << ", " << synapsePos.z( )
              << std::endl;

    std::cout << "Node 0 " << nodes.firstPosition( ).x( )
               << ", " << nodes.firstPosition( ).y( )
               << ", " << nodes.firstPosition( ).z( )
               << std::endl;

    for( unsigned int i = 0; i < nodes.size( ) - 1; ++i )
    {
      currentPoint = nodes[ i ];
      nextPoint = nodes[ i + 1 ];

      std::cout << "Node " << i + 1 << " " << nextPoint.x( )
                 << ", " << nextPoint.y( )
                 << ", " << nextPoint.z( )
                 << std::endl;

      ab = nextPoint - currentPoint;
      ac = synapsePos - currentPoint;

      kAC = ab.dot( ac );
      kAB = ab.dot( ab );

      if( kAC == 0 || ( kAC > 0 && kAC < kAB ))
      {
        index = i;
        found = true;
        break;
      }
      else if( kAC == kAB )
      {
        index = i + 1;
        found = true;
        break;
      }

    }

    vec3 projectedPos( 0, 0, 0 );

    if( found )
    {
      projectedDist = kAC / nodes.segmentDistance( index );

      projectedPos =
          nodes[ index ]  + nodes.segmentDirection( index ) * projectedDist;

      std::cout << "Initial pos " << synapsePos.x( )
                                << ", " << synapsePos.y( )
                                << ", " << synapsePos.z( )
              << " Projected pos " << projectedPos.x( )
                            << ", " << projectedPos.y( )
                            << ", " << projectedPos.z( )
                            << std::endl;

    }

    return std::make_tuple( projectedPos, projectedDist, index, found );

//      if( ab.cross( ac ).norm( ) != 0.0f )
//        continue;


  }



}


