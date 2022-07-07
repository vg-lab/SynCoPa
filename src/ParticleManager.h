//
// Created by gaeqs on 9/06/22.
//

#ifndef SYNCOPA_PARTICLEMANAGER_H
#define SYNCOPA_PARTICLEMANAGER_H

#include "particlelab/SynapseParticle.h"
#include "particlelab/StaticModel.h"
#include "particlelab/StaticGradientModel.h"
#include "particlelab/DynamicPathParticle.h"
#include "particlelab/DynamicModel.h"

#include <plab/core/Cluster.h>
#include <reto/ShaderProgram.h>
#include <nlgeometry/AxisAlignedBoundingBox.h>

namespace syncopa
{

  class ParticleManager
  {

    nlgeometry::AxisAlignedBoundingBox _particlesBoundingBox;

    // GLOBAL COMPONENTS
    reto::ShaderProgram _staticProgram;
    reto::ShaderProgram _staticGradientProgram;
    reto::ShaderProgram _dynamicProgram;
    std::shared_ptr< plab::Renderer > _staticRenderer;
    std::shared_ptr< plab::Renderer > _staticGradientRenderer;
    std::shared_ptr< plab::Renderer > _dynamicRenderer;

    // SYNAPSES
    std::shared_ptr< plab::Cluster< SynapseParticle >> _synapseCluster;
    std::shared_ptr< StaticModel > _synapseModel;
    std::shared_ptr< StaticGradientModel > _synapseGradientModel;
    nlgeometry::AxisAlignedBoundingBox _synapseBB;

    // PATHS
    std::shared_ptr< plab::Cluster< SynapseParticle >> _pathCluster;
    std::shared_ptr< StaticModel > _pathModel;
    nlgeometry::AxisAlignedBoundingBox _pathBB;

    // DYNAMIC
    std::shared_ptr< plab::Cluster< DynamicPathParticle>> _dynamicCluster;
    std::shared_ptr< DynamicModel > _dynamicModel;

    void recalculateParticlesBoundingBox( );

  public:

    ParticleManager( const ParticleManager& ) = delete;

    ParticleManager( );

    void init( std::shared_ptr< plab::ICamera > camera );

    const std::shared_ptr< StaticModel >& getSynapseModel( ) const;

    const std::shared_ptr< StaticGradientModel >&
    getSynapseGradientModel( ) const;

    const std::shared_ptr< StaticModel >& getPathModel( ) const;

    const std::shared_ptr< DynamicModel >& getDynamicModel( ) const;

    void setAccumulativeMode( bool accumulativeMode );

    const nlgeometry::AxisAlignedBoundingBox& getSynapseBoundingBox( ) const;

    const nlgeometry::AxisAlignedBoundingBox& getPathBoundingBox( ) const;

    const nlgeometry::AxisAlignedBoundingBox& getParticlesBoundingBox( ) const;

    void setSynapses( const tsynapseVec& synapses );

    void setMappedSynapses( const tsynapseVec& synapses ,
                            const tFloatVec& lifeValues );

    void setPaths(
      const std::vector< vec3 >& pre , const std::vector< vec3 >& post );

    void setDynamic( const std::vector< DynamicPathParticle >& particles );

    void clearSynapses( );

    void clearPaths( );

    void clearDynamic( );

    void draw( bool drawPaths, bool drawDynamic  ) const;

  };

}


#endif //SYNCOPA_PARTICLEMANAGER_H
