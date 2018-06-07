/*
 * @file  Scene.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_SCENE_CPP_
#define SRC_SCENE_CPP_

#include "NeuronScene.h"

namespace synvis
{


  NeuronScene::NeuronScene( nsol::DataSet* dataset )
  : _dataset( dataset )
  {
    _attribsFormat.resize( 3 );
    _attribsFormat[0] = nlgeometry::TAttribType::POSITION;
    _attribsFormat[1] = nlgeometry::TAttribType::CENTER;
    _attribsFormat[2] = nlgeometry::TAttribType::TANGENT;

  }

  NeuronScene::~NeuronScene( void )
  {
    unload( );
  }

  void NeuronScene::unload( void )
  {
    if( _dataset )
      delete _dataset;
  }

  void NeuronScene::clear( void )
  {
    if( _dataset )
      _dataset->close( );
  }


  void NeuronScene::generateMeshes( void )
  {
    std::cout << "Generating " << _dataset->neurons( ).size( ) << std::endl;
    for ( auto neuronIt: _dataset->neurons( ))
    {
      auto morphology = neuronIt.second->morphology( );
      auto morphoIt = _neuronMeshes.find( morphology );
      if( morphoIt == _neuronMeshes.end( ))
      {
        auto simplifier = nsol::Simplifier::Instance( );
        simplifier->adaptSoma( morphology );
        simplifier->simplify( morphology, nsol::Simplifier::DIST_NODES_RADIUS );

        auto mesh = nlgenerator::MeshGenerator::generateMesh( morphology );
        mesh->uploadGPU( _attribsFormat, nlgeometry::Facet::PATCHES );
        mesh->clearCPUData( );
        _neuronMeshes[ morphology ] = mesh;

      }
      _neuronMorphologies[ neuronIt.first ] = morphology;
    }
  }

  TRenderMorpho NeuronScene::getRender( const std::set< unsigned int >& gids_ ) const
  {
    if( gids_.empty( ))
      return TRenderMorpho( );

    std::vector< unsigned int > gids;
    std::vector< nlgeometry::MeshPtr > meshes;
    std::vector< mat4 > matrices;
    std::vector< vec3 > colors;

    unsigned int totalMeshes = gids_.size( );

    gids.reserve( totalMeshes );
    meshes.reserve( totalMeshes );
    matrices.reserve( totalMeshes );
    colors.resize( totalMeshes, vec3( 0, 0, 0 ));

    nsol::NeuronsMap& neurons = _dataset->neurons();

    for( auto gid : gids_ )
    {
      auto neuron = neurons.find( gid );
      auto morphology = _neuronMorphologies.find( gid );
      if( morphology != _neuronMorphologies.end( ))
      {
        gids.push_back( gid );
        auto meshIt = _neuronMeshes.find( morphology->second );
  //      if( meshIt != _neuronMeshes.end( ))
        meshes.push_back( meshIt->second );
  //      auto morphIt = _neuronMorphologies.find( gid );
//        colors.push_back( _colorPre );
  //      if( morphIt != _neuronMorphologies.end( ))
  //      {
        auto matrix = neuron->second->transform( );
        matrices.push_back( matrix );
      }
      else
        std::cout << "Could not find " << gid << " morphology " << std::endl;
    }

    return std::make_tuple( gids, meshes, matrices, colors );
  }

//  TRenderMorpho NeuronScene::getRender( const std::vector< unsigned int >& gidsPre,
//                                        const std::vector< unsigned int >& gidsPost ) const
//  {
//    std::vector< nlgeometry::MeshPtr > meshes;
//    std::vector< mat4 > matrices;
//    std::vector< vec3 > colors;
//
//    unsigned int totalMeshes = gidsPre.size( ) + gidsPost.size( );
//    meshes.reserve( totalMeshes );
//    matrices.reserve( totalMeshes);
//    colors.reserve( totalMeshes );
//
////    colors.resize( gidsPre.size( ), Eigen::Vector3f::Ones( ));
//
//    std::cout << "Creating render of " << gidsPre.size( ) << std::endl;
//    nsol::NeuronsMap& neurons = _dataset->neurons();
//
//    for( auto gid : gidsPre )
//    {
//      auto neuron = neurons.find( gid );
//      auto morphology = _neuronMorphologies.find( gid );
//      if( morphology != _neuronMorphologies.end( ))
//      {
//        auto meshIt = _neuronMeshes.find( morphology->second );
//  //      if( meshIt != _neuronMeshes.end( ))
//        meshes.push_back( meshIt->second );
//  //      auto morphIt = _neuronMorphologies.find( gid );
//        colors.push_back( _colorPre );
//  //      if( morphIt != _neuronMorphologies.end( ))
//  //      {
//        auto matrix = neuron->second->transform( );
//        matrices.push_back( matrix );
//      }
//      else
//        std::cout << "Could not find " << gid << " morphology " << std::endl;
//    }
//
//    for( auto gid : gidsPost )
//    {
//      auto neuron = neurons.find( gid );
//      auto morphology = _neuronMorphologies.find( gid );
//      if( morphology != _neuronMorphologies.end( ))
//      {
//        auto meshIt = _neuronMeshes.find( morphology->second );
//  //      if( meshIt != _neuronMeshes.end( ))
//        meshes.push_back( meshIt->second );
//  //      auto morphIt = _neuronMorphologies.find( gid );
//        colors.push_back( _colorPost );
//  //      if( morphIt != _neuronMorphologies.end( ))
//  //      {
//        auto matrix = neuron->second->transform( );
//        matrices.push_back( matrix );
//      }
//      else
//        std::cout << "Could not find " << gid << " morphology " << std::endl;
//    }
//
//    std::cout << "Finished render config." << std::endl;
//
//    std::vector< unsigned int > gids;
//    gids.insert( gids.end( ), gidsPre.begin( ), gidsPre.end( ));
//    gids.insert( gids.end( ), gidsPost.begin( ), gidsPost.end( ));
//
//    return std::make_tuple( gids, meshes, matrices, colors );
//  }

  void NeuronScene::computeBoundingBox( std::vector< unsigned int > indices_ )
  {
    Eigen::Array3f minimum =
        Eigen::Array3f::Constant( std::numeric_limits< float >::max( ));
    Eigen::Array3f maximum =
        Eigen::Array3f::Constant( std::numeric_limits< float >::min( ));

    for ( const auto id: indices_ )
    {
      auto neuronMapIt = _dataset->neurons( ).find( id );
      if ( neuronMapIt != _dataset->neurons( ).end( ))
      {
        auto neuron = neuronMapIt->second;
        auto morphology = neuron->morphology( );
        if ( morphology )
        {
          auto radius = morphology->soma( )->maxRadius( );
          auto center = morphology->soma( )->center( );
          Eigen::Vector4f position = neuron->transform( ) *
              nsol::Vec4f( center.x( ) , center.y( ), center.z( ), 1.0f );
          Eigen::Array3f minVec( position.x( ) - radius, position.y( ) - radius,
                                 position.z( ) - radius );
          Eigen::Array3f maxVec( position.x( ) + radius, position.y( ) + radius,
                                 position.z( ) + radius );
          minimum = minimum.min( minVec );
          maximum = maximum.max( maxVec );
        }
      }
    }

    _boundingBox = nlgeometry::AxisAlignedBoundingBox( minimum, maximum );
  }

  void NeuronScene::computeBoundingBox( void )
  {
    std::vector< unsigned int > indices;
    for ( const auto& neuronIt: _dataset->neurons( ))
    {
      indices.push_back( neuronIt.first );
    }

    computeBoundingBox( indices );
  }

  nlgeometry::AxisAlignedBoundingBox NeuronScene::boundingBox( void ) const
  {
    return _boundingBox;
  }


  void NeuronScene::color( const vec3& color_, TNeuronConnection type )
  {
    if( type == PRESYNAPTIC )
      _colorPre = color_;
    else if( type == POSTSYNAPTIC )
      _colorPost = color_;
    else
    {
      _colorPre = color_;
      _colorPost = color_;
    }

  }


}

#endif /* SRC_SCENE_CPP_ */
