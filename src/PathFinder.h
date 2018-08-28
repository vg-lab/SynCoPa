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
    tpi_distanceToSynapse,
    tpi_index,
    tpi_found
  };

  enum tSynapseInfo
  {
    tsy_fixedPosition = 0,
    tsy_distanceOnSection,
    tsy_distanceToSynapse,
    tsy_indexLastNode
  };

  enum tSynapseFixInfo
  {
    tsfi_pointer = 0,
    tsfi_section,
    tsfi_node,
    tsfi_distance
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

  typedef std::unordered_map< nsol::MorphologySynapsePtr, std::pair<
      nsol::NeuronMorphologySectionPtr, nsol::NeuronMorphologySectionPtr >> tSynapseFixedSections;

  typedef std::tuple< nsolMSynapse_ptr, unsigned int, unsigned int, float > tBrainSynapse;

  class PathFinder
  {
  public:

    PathFinder( nsol::DataSet* dataset = nullptr );
    ~PathFinder( void );

    void dataset( nsol::DataSet* dataset_ );

    void clear( void );

    const std::vector< nsolMSynapse_ptr >& getSynapses( void ) const;
//    std::vector< nsolMSynapse_ptr > getSynapses( unsigned int gidPre ) const;
//    std::vector< nsolMSynapse_ptr > getSynapses( const std::set< unsigned int >& gidsPre ) const;

    mat4 getTransform( unsigned int gid ) const;

    std::set< unsigned int > connectedTo( unsigned int gid ) const;

    std::vector< vec3 > getAllPathsPoints( unsigned int gid,
                                           const std::set< unsigned int >& gidsPost,
                                           float pointSize,
                                           TNeuronConnection type = PRESYNAPTIC ) const;

    tSectionsInfoMap processSections( const std::vector< nsolMSynapse_ptr >& synapses,
                                      TNeuronConnection type,
                                      unsigned int gid ) const;

    tSectionsInfoMap parseSections( const std::vector< nsolMSynapse_ptr >& synapses,
                                    TNeuronConnection type = PRESYNAPTIC ) const;


    std::vector< nsolMSection_ptr > pathToSoma( nsolMSection_ptr section ) const;
    std::vector< nsolMSection_ptr > pathToSoma( const nsolMSynapse_ptr synapse,
                                                TNeuronConnection type = PRESYNAPTIC ) const;

    void configure( unsigned int presynapticGid );

    void addPostsynapticPath( nsolMSynapse_ptr synapse,
                              const tPosVec& nodes );

    const ConnectivityTree& tree( void ) const;

    cnode_ptr node( unsigned int sectionID ) const;

    void computedPathFrom( unsigned int sectionID, const utils::EventPolylineInterpolation& path );
    utils::EventPolylineInterpolation computedPathFrom( unsigned int sectionID );

  protected:

    void _loadSynapses( unsigned int presynapticGID );

    unsigned int findSynapseSegment(  const vec3& synapsePos,
                                      const nsol::Nodes& nodes ) const;

    tSectionsMap findEndSections( const std::vector< nsolMSynapse_ptr >& synapses,
                                  TNeuronConnection type = PRESYNAPTIC ) const;

    tFixedSynapseInfo projectSynapse( const vec3 synapsePos,
                                      const utils::PolylineInterpolation& nodes ) const;

    tFixedSynapseInfo findClosestPointToSynapse( const vec3 synapsePos,
                                                 const utils::PolylineInterpolation& nodes ) const;



    std::vector< vec3 > cutEndSection( const std::vector< vec3 >& nodes,
                                       const vec3&  synapsePos,
                                       unsigned int index ) const;

    nsol::DataSet* _dataset;

    std::unordered_map< unsigned int , tSynapseFixedSections> _fixedSynapseSections;

    ConnectivityTree _treePre;
    std::unordered_map< unsigned int, ConnectivityTree > _treePost;

    tSectionsInfoMap _infoSections;

    std::unordered_map< unsigned int, utils::EventPolylineInterpolation > _computedPaths;

    std::unordered_map< nsol::MorphologySynapsePtr,
                        utils::PolylineInterpolation > _pathsPost;


    std::vector< nsolMSynapse_ptr > _synapses;

    std::unordered_map< nsolMSynapse_ptr, std::pair< tBrainSynapse, tBrainSynapse >> _synapseFixInfo;

    unsigned int _presynapticGID;

    unsigned int _maxDepth;
  };
}



#endif /* PATHFINDER_H_ */
