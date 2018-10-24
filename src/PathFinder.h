/*
 * @file	PathFinder.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef PATHFINDER_H_
#define PATHFINDER_H_

#include "types.h"

#include <unordered_set>

#include <nsol/nsol.h>

#include "PolylineInterpolation.hpp"
#include "ConnectivityTree.h"

namespace syncopa
{
  enum tSectionInfoFields
  {
    tsi_Synapses = 0,
    tsi_length,
    tsi_leafSection,
    tsi_fixedSection,
    tsi_Interpolator
  };

  enum tProjectionInfo
  {
    tpi_position = 0,
    tpi_distance,
    tpi_distanceOnSection,
    tpi_index,
    tpi_found
  };

  enum TBrainSynapse
  {
    TBS_SECTION_ID = 0,
    TBS_SEGMENT_INDEX,
    TBS_SEGMENT_DISTANCE
  };

  enum TBrainSynapseAttribs
  {
    TBSA_SYNAPSE_DELAY = 0,
    TBSA_SYNAPSE_CONDUCTANCE,
    TBSA_SYNAPSE_UTILIZATION,
    TBSA_SYNAPSE_DEPRESSION,
    TBSA_SYNAPSE_FACILITATION,
    TBSA_SYNAPSE_DECAY,
    TBSA_SYNAPSE_EFFICACY,
    TBSA_SYNAPSE_OTHER
  };

  enum TBrainSynapseInfo
  {
    TBSI_ATTRIBUTES = 0,
    TBSI_PRESYNAPTIC,
    TBSI_POSTSYNAPTIC
  };



  typedef std::tuple< vec3, float, float, unsigned int, bool > tFixedSynapseInfo;

  typedef std::tuple< std::unordered_map< nsol::MorphologySynapsePtr,
                      tFixedSynapseInfo>,
                      float,
                      bool,
                      std::vector< vec3 >,
                      utils::PolylineInterpolation > tSectionInfo;

  typedef std::unordered_map< nsol::NeuronMorphologySectionPtr,
                              tSectionInfo > tSectionsInfoMap;

  typedef std::unordered_set< nsol::NeuronMorphologySectionPtr > tSectionsMap;

  typedef std::tuple< unsigned int, unsigned int, float > tBrainSynapse;

  typedef std::tuple< float, float, float, float, float, float, int > tBrainSynapseAttribs;

  typedef std::unordered_map< nsolMSynapse_ptr,
      std::tuple< tBrainSynapseAttribs, tBrainSynapse, tBrainSynapse >> TSynapseInfo;

  class PathFinder
  {
  public:

    PathFinder( nsol::DataSet* dataset = nullptr );
    ~PathFinder( void );

    void dataset( nsol::DataSet* dataset_ );

    void loadSynapses( const gidUSet& gids );

    void configure( unsigned int presynapticGid, const gidUSet& postsynapticGIDS );

    void clear( void );

    const std::vector< nsolMSynapse_ptr >& getSynapses( void ) const;
//    std::vector< nsolMSynapse_ptr > getSynapses( unsigned int gidPre ) const;
//    std::vector< nsolMSynapse_ptr > getSynapses( const std::set< unsigned int >& gidsPre ) const;

    mat4 getTransform( unsigned int gid ) const;

    gidUSet connectedTo( unsigned int gid ) const;

    std::vector< vec3 > getAllPathsPoints( unsigned int gid,
                                           const gidUSet& gidsPost,
                                           float pointSize,
                                           TNeuronConnection type = PRESYNAPTIC ) const;
//
//    tSectionsInfoMap parseSections( const std::vector< nsolMSynapse_ptr >& synapses,
//                                    TNeuronConnection type = PRESYNAPTIC ) const;


    std::vector< nsolMSection_ptr > pathToSoma( nsolMSection_ptr section ) const;
    std::vector< nsolMSection_ptr > pathToSoma( const nsolMSynapse_ptr synapse,
                                                TNeuronConnection type = PRESYNAPTIC ) const;

    void addPostsynapticPath( nsolMSynapse_ptr synapse,
                              const tPosVec& nodes );

    utils::EventPolylineInterpolation getPostsynapticPath( nsolMSynapse_ptr synapse ) const;

    const ConnectivityTree& presynapticTree( void ) const;

    cnode_ptr node( unsigned int sectionID ) const;

    void computedPathFrom( unsigned int sectionID, const utils::EventPolylineInterpolation& path );
    utils::EventPolylineInterpolation computeDeepestPathFrom( unsigned int sectionID );

    const TSynapseInfo& synapsesInfo( void ) const;

    std::vector< vec3 > cutLeafSection( unsigned int sectionID ) const;

  protected:

    void _loadSynapses( unsigned int presynapticGID,
                        const gidUSet& postsynapticGIDs );

    void _populateTrees( const tsynapseVec& synapses );


    void _processSections( const std::vector< nsolMSynapse_ptr >& synapses,
                           unsigned int presynapticGID,
                           const gidUSet& neuronGIDs );

    void _processEndSections( unsigned int gidPresynaptic, const gidUSet& gidsPost );

    unsigned int findSynapseSegment(  const vec3& synapsePos,
                                      const nsol::Nodes& nodes ) const;


    std::vector< vec3 > _cutEndSection( const std::vector< vec3 >& nodes,
                                       const vec3&  synapsePos,
                                       unsigned int index ) const;

    nsol::DataSet* _dataset;

    ConnectivityTree _treePre;
    std::unordered_map< unsigned int, ConnectivityTree > _treePost;

    tSectionsInfoMap _infoSections;

    std::unordered_map< unsigned int, utils::EventPolylineInterpolation > _computedPaths;

    std::unordered_map< nsolMSynapse_ptr,
                        utils::PolylineInterpolation > _pathsPost;

    std::unordered_set< nsolMSynapse_ptr > _somaSynapses;

    std::vector< nsolMSynapse_ptr > _synapses;

    TSynapseInfo _synapseFixInfo;

    unsigned int _presynapticGID;

    unsigned int _maxDepth;
  };
}



#endif /* PATHFINDER_H_ */
