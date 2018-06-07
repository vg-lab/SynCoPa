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

namespace synvis
{

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
        if( type == POSTSYNAPTIC )
          currentGid = syn->postSynapticNeuron( );

        auto transform = getTransform( currentGid );

        auto msyn = dynamic_cast< nsol::MorphologySynapsePtr >( syn );
        if( !msyn )
          std::cerr << "Error converting from Synapse to MorphologySynapse " << syn << std::endl;

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

//          if( counter == 0 )
//          {
//            std::cout << "Section " << counter << " nodes " << nodes.size( ) << std::endl;
//
//            unsigned int closestPoint = nodes.size( ) - 1;
//            float minDist = std::numeric_limits< float >::max( );
//
//            vec3 synPos = msyn->preSynapticSurfacePosition( );
//
//            std::cout << "Distances: ";
//            unsigned int idx = 0;
//            vec3 initPos = nodes.front( )->point( );
//            float synDist = ( synPos - initPos ).norm( );
//            for( auto node : nodes )
//            {
//              if( idx == 0 )
//                continue;
//
//              float distance = ( node->point( ) - initPos ).norm( );
//              if( distance <= synDist )
//              {
//                std::cout << " " << idx << ": " << distance;
//                closestPoint = idx;
//                minDist = distance;
//
//              }
//              else
//                break;
//
//              ++idx;
//            }
//            std::cout << std::endl;
//
//            std::cout << "Closest distance " << minDist << " from node " << closestPoint << std::endl;
//
//            for( unsigned int i = 0; i < closestPoint + 1; ++i )
//              positions.push_back( nodes[ i ]->point( ));
//
//          }
//          else
//          {
//            positions.reserve( nodes.size( ));
//            for( auto node : nodes )
//              positions.push_back( node->point( ));

//          }

          float accDist = 0;

          vec3 currentPoint;
          vec3 nextPoint;

          utils::PolylineInterpolation pathPoints;
          pathPoints.insert( 0, nodes.back( )->point( ), vec3( 0, 0, 0 ));

          for( unsigned int i = nodes.size( ) - 1; i > 1; --i )
          {
            currentPoint = nodes[ i ]->point( );
            nextPoint = nodes[ i - 1 ]->point( );

            vec3 dir = nextPoint - currentPoint;
            float module = dir.norm( );
            accDist += module;
            dir = dir / module;

            pathPoints.insert( accDist, nextPoint, dir );
          }

          float currentDist = 0;

          while( currentDist < accDist )
          {
            vec3 pos = pathPoints.pointAtDistance( currentDist );

            vec4 transPoint( pos.x( ), pos.y( ), pos.z( ), 1 );
            transPoint = transform * transPoint;

            result.push_back( transPoint.block< 3, 1 >( 0, 0  ));

            currentDist += pointSize;
          }

          ++counter;
      }
    }
    }


//    auto transformIt = transforms.begin( );
//    std::vector< vec3 > paths;
//    for( const auto& path : presynapticPaths )
//    {
//      for( auto node : path )
//      {
//        auto nodepos = node->point( );
//        ;
//        vec4 point = *transformIt * vec4( nodepos.x( ), nodepos.y( ), nodepos.z( ), 1 ) ;
//        paths.push_back( vec3( point.x( ), point.y( ), point.z( )));
//  //      paths.push_back( node->point( ) );
//      }
//      ++transformIt;
//    }


    return result;

  }


  std::vector< nsol::NeuronMorphologySectionPtr > PathFinder::findPathToSoma( const nsol::MorphologySynapsePtr synapse,
                                          synvis::TNeuronConnection type ) const
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

}


