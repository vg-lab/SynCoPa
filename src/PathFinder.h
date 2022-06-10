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
    tsi_Synapses = 0 ,
    tsi_length ,
    tsi_leafSection ,
    tsi_fixedSection ,
    tsi_Interpolator
  };

  enum tProjectionInfo
  {
    tpi_position = 0 ,
    tpi_distance ,
    tpi_distanceOnSection ,
    tpi_index ,
    tpi_found
  };

  typedef std::tuple< vec3 , float , float , unsigned int , bool > tFixedSynapseInfo;

  typedef std::tuple< std::unordered_map< nsol::MorphologySynapsePtr ,
    tFixedSynapseInfo > ,
    float ,
    bool ,
    std::vector< vec3 > ,
    utils::PolylineInterpolation > tSectionInfo;

  typedef std::unordered_map< nsol::NeuronMorphologySectionPtr ,
    tSectionInfo > tSectionsInfoMap;

  typedef std::unordered_set< nsol::NeuronMorphologySectionPtr > tSectionsMap;

  class PathFinder
  {
  public:

    PathFinder( void );

    ~PathFinder( void );

    void dataset( nsol::DataSet* dataset_ , const TSynapseInfo* synapseInfo );

    void configure( const std::vector< nsol::SynapsePtr >& synapses ,
                    const std::unordered_set< unsigned int >& preNeuronsWithAllPaths ,
                    const std::unordered_set< unsigned int >& postNeuronsWithAllPaths ,
                    const std::unordered_set< unsigned int >& preNeuronsWithConnectedPaths ,
                    const std::unordered_set< unsigned int >& postNeuronsWithConnectedPaths ,
                    float pointSize ,
                    std::vector< vec3 >& preOut ,
                    std::vector< vec3 >& postOut );

    void clear( void );

    std::vector< nsolMSection_ptr >
    pathToSoma( nsolMSection_ptr section ) const;

    std::vector< nsolMSection_ptr > pathToSoma( const nsolMSynapse_ptr synapse ,
                                                TNeuronConnection type = PRESYNAPTIC ) const;

    void addPostsynapticPath( nsolMSynapse_ptr synapse ,
                              const tPosVec& nodes );

    utils::EventPolylineInterpolation
    getPostsynapticPath( nsolMSynapse_ptr synapse ) const;

    const std::unordered_map< unsigned int , ConnectivityTree >&
    presynapticTrees( void ) const;

    std::pair< unsigned int , cnode_ptr > node( unsigned int sectionID ) const;

    void computedPathFrom( unsigned int sectionID ,
                           const utils::EventPolylineInterpolation& path );

    utils::EventPolylineInterpolation
    computeDeepestPathFrom( unsigned int sectionID );

    std::vector< vec3 > cutLeafSection( unsigned int sectionID ) const;

    mat4 getTransform( unsigned int gid ) const;

  protected:

    void _calculateSynapses(
      const std::vector< nsol::SynapsePtr >& synapses ,
      const std::unordered_set< unsigned int >& preNeuronsWithAllPaths ,
      const std::unordered_set< unsigned int >& postNeuronsWithAllPaths ,
      const std::unordered_set< unsigned int >& preNeuronsWithConnectedPaths ,
      const std::unordered_set< unsigned int >& postNeuronsWithConnectedPaths ,
      tsynapseVec& outUsedSynapses ,
      tsynapseVec& outUsedPreSynapses ,
      tsynapseVec& outUsedPostSynapses ) const;


    void _createPath(
      std::unordered_set< nsol::NeuronMorphologySectionPtr >& insertedSections ,
      std::vector< vec3 >& result ,
      nsol::MorphologySynapse* syn ,
      TNeuronConnection type ,
      float pointSize ) const;

    void _populateTrees( const tsynapseVec& preSynapses,
                         const tsynapseVec& postSynapses );


    void _processSections( const std::vector< nsolMSynapse_ptr >& preSynapses ,
                           const std::vector< nsolMSynapse_ptr >& postSynapses );

    void
    _processEndSections( const std::vector< nsolMSynapse_ptr >& preSynapses ,
                         const std::vector< nsolMSynapse_ptr >& postSynapses );

    unsigned int findSynapseSegment( const vec3& synapsePos ,
                                     const nsol::Nodes& nodes ) const;


    std::vector< vec3 > _cutEndSection( const std::vector< vec3 >& nodes ,
                                        const vec3& synapsePos ,
                                        unsigned int index ) const;

    nsol::DataSet* _dataset;

    const TSynapseInfo* _synapseFixInfo;

    std::unordered_map< unsigned int , ConnectivityTree > _treePre;
    std::unordered_map< unsigned int , ConnectivityTree > _treePost;

    tSectionsInfoMap _infoSections;

    std::unordered_map< unsigned int , utils::EventPolylineInterpolation > _computedPaths;

    std::unordered_map< nsolMSynapse_ptr , utils::PolylineInterpolation > _pathsPre;
    std::unordered_map< nsolMSynapse_ptr , utils::PolylineInterpolation > _pathsPost;

    std::unordered_set< nsolMSynapse_ptr > _somaSynapses;

    unsigned int _maxDepth;
  };
}


#endif /* PATHFINDER_H_ */
