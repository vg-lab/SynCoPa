/*
 * @file  Scene.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_SCENE_H_
#define SRC_SCENE_H_

#include "types.h"

#include <nsol/nsol.h>

#include <nlgenerator/nlgenerator.h>
#include <nlgeometry/nlgeometry.h>

namespace synvis
{
  struct MeshConfig
  {
  public:

    nlgeometry::Mesh mesh;
    mat4 transform;

    vec3 color;

  };

  enum TRenderEnum
  {
    ID = 0,
    MESH,
    MATRIX,
    COLOR
  };

  typedef std::tuple< std::vector< unsigned int >,
                      std::vector< nlgeometry::MeshPtr >,
                      std::vector< mat4 >,
                      std::vector< vec3 >> TRenderMorpho;

  class Scene
  {
public:

    Scene( nsol::DataSet* dataset );

    ~Scene( void );

    void clear( void );

    void unload( void );

    void generateMeshes( void );

    TRenderMorpho getRender( const std::vector< unsigned int >& gids );

    void computeBoundingBox( std::vector< unsigned int > indices_ );

    void computeBoundingBox( void );

    nlgeometry::AxisAlignedBoundingBox boundingBox( void );


protected:

    nsol::DataSet* _dataset;

    nlgeometry::AxisAlignedBoundingBox _boundingBox;

    //! Meshes attribs format
    nlgeometry::AttribsFormat _attribsFormat;

    std::unordered_map< nsol::NeuronMorphologyPtr, nlgeometry::MeshPtr > _neuronMeshes;
    std::unordered_map< unsigned int, nsol::NeuronMorphologyPtr > _neuronMorphologies;

  };


}




#endif /* SRC_SCENE_H_ */
