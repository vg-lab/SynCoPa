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

#include <unordered_set>
#include <GL/glew.h>

namespace syncopa
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
    std::unordered_set< nsol::NeuronMorphologyPtr > morphologies;
    std::vector< nsol::NeuronMorphologyPtr > vecMorpho;

    for (const auto &neuronIt: _dataset->neurons( ))
    {
      const auto morphology = neuronIt.second->morphology( );
      const auto morphoIt = morphologies.find( morphology );
      if( morphoIt == morphologies.end( ))
      {
        morphologies.insert( morphology );
        vecMorpho.push_back( morphology );
      }

      _neuronMorphologies[ neuronIt.first ] = morphology;
    }

    auto reportValue = [this](const unsigned int value)
    {
      #pragma omp single nowait
      {
        static unsigned int oldProgress = 0;

        if(oldProgress < value)
        {
          oldProgress = value;

          emit progress("Generating meshes", oldProgress);
        }
      }
    };

    unsigned int count = 0;
    #pragma omp parallel for shared(count)
    for( int i = 0; i < static_cast<int>(vecMorpho.size()); ++i )
    {
      auto morphology = vecMorpho[ i ];

      auto simplifier = nsol::Simplifier::Instance( );
      simplifier->adaptSoma( morphology );
      simplifier->simplify( morphology, nsol::Simplifier::DIST_NODES_RADIUS );

      auto mesh = nlgenerator::MeshGenerator::generateMesh( morphology );
      _neuronMeshes[ morphology ] = mesh;

      #pragma omp atomic
      ++count;

      reportValue(count*100/vecMorpho.size());
    }

    emit progress("Generated meshes", 100);
  }

  TRenderMorpho NeuronScene::getRender( const gidUSet& gids_ ) const
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
        const auto meshIt = _neuronMeshes.find( morphology->second );
        assert( meshIt != _neuronMeshes.end( ));
        meshes.push_back( meshIt->second );
        const auto matrix = neuron->second->transform( );
        matrices.push_back( matrix );
      }
      else
        std::cout << "Could not find " << gid << " morphology " << std::endl;
    }

    return std::make_tuple( gids, meshes, matrices, colors );
  }

  void NeuronScene::computeBoundingBox( const gidVec& indices_ )
  {
    Eigen::Array3f minimum = Eigen::Array3f::Constant( std::numeric_limits< float >::max( ));
    Eigen::Array3f maximum = Eigen::Array3f::Constant( std::numeric_limits< float >::min( ));

    for ( const auto id: indices_ )
    {
      auto neuronMapIt = _dataset->neurons( ).find( id );
      if ( neuronMapIt != _dataset->neurons( ).end( ))
      {
        const auto neuron = neuronMapIt->second;
        const auto morphology = neuron->morphology( );
        if ( morphology )
        {
          const auto radius = morphology->soma( )->maxRadius( );
          const auto center = morphology->soma( )->center( );
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
    gidVec indices;
    auto addIndex = [&indices](const std::pair<unsigned int, nsol::NeuronPtr> &n)
    {
      indices.push_back(n.first);
    };
    const auto &neurons = _dataset->neurons();
    std::for_each(neurons.cbegin(), neurons.cend(), addIndex);

    computeBoundingBox( indices );
  }

  nlgeometry::AxisAlignedBoundingBox NeuronScene::boundingBox( void ) const
  {
    return _boundingBox;
  }

  void NeuronScene::color( const vec3& color_, TNeuronConnection type )
  {
    switch(type)
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _colorPre = color_;
        if(type == TNeuronConnection::PRESYNAPTIC) return;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _colorPost = color_;
    }
  }

  void NeuronScene::uploadMeshes()
  {
    const auto total = _neuronMeshes.size();
    unsigned int count = 0;

    auto emitProgress = [this](const unsigned int p)
    {
      static unsigned int oldProgress = 0;
      if(oldProgress < p)
      {
        oldProgress = p;
        emit progress("Uploading meshes to GPU", oldProgress);
      }
    };

    for( auto &mesh : _neuronMeshes )
    {
      mesh.second->uploadGPU( _attribsFormat, nlgeometry::Facet::PATCHES );
      mesh.second->clearCPUData( );

      ++count;
      emitProgress((count*100)/total);
    }
  }
}

#endif /* SRC_SCENE_CPP_ */
