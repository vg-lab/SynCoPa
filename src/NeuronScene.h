/*
 * @file  Scene.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_NEURONSCENE_H_
#define SRC_NEURONSCENE_H_

#include "types.h"

#include <nsol/nsol.h>

#include <nlgenerator/nlgenerator.h>
#include <nlgeometry/nlgeometry.h>
#include <boost/thread.hpp>
#include <boost/thread/concurrent_queues/sync_bounded_queue.hpp>

namespace syncopa
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
    ID = 0 ,
    MESH ,
    MATRIX ,
    COLOR ,
    SHOW_SOMA ,
    SHOW_MORPHOLOGIES
  };

  enum TEgoNetworkEnum
  {
    EGO_COLOR = 0 ,
    EGO_SHOW_MORPHS ,
    EGO_ENABLED ,
  };

  typedef std::tuple< std::vector< unsigned int > ,
    std::vector< nlgeometry::MeshPtr > ,
    std::vector< mat4 > ,
    std::vector< vec3 > ,
    bool ,
    bool > TRenderMorpho;

  class NeuronScene
    : public QObject
  {
  Q_OBJECT
  public:
    NeuronScene( nsol::DataSet* dataset );

    ~NeuronScene( void );

    void clear( void );

    void unload( void );

    void generateMeshes( void );

    void flushMeshesOnCPU( );

    TRenderMorpho getRender( const gidUSet& gids ) const;

    void computeBoundingBox( const gidVec& indices_ );

    void computeBoundingBox( void );

    nlgeometry::AxisAlignedBoundingBox boundingBox( void ) const;

    void color( const vec3& color_ , TNeuronConnection type );

  signals:

    void progress( const QString& message , const unsigned int value );

  protected:
    nsol::DataSet* _dataset;

    nlgeometry::AxisAlignedBoundingBox _boundingBox;

    //! Meshes attribs format
    nlgeometry::AttribsFormat _attribsFormat;

    std::unordered_map< nsol::NeuronMorphologyPtr , nlgeometry::MeshPtr > _neuronMeshes;
    std::unordered_map< unsigned int , nsol::NeuronMorphologyPtr > _neuronMorphologies;
    boost::sync_bounded_queue< nlgeometry::MeshPtr > _meshesOnCPU;

    vec3 _colorPre;
    vec3 _colorPost;
  };
}

#endif /* SRC_NEURONSCENE_H_ */
