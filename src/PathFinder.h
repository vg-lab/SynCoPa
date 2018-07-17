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

  class PathFinder
  {
  public:

    PathFinder( nsol::DataSet* dataset = nullptr );
    ~PathFinder( void );

    void dataset( nsol::DataSet* dataset_ );

    mat4 getTransform( unsigned int gid ) const;

    std::set< unsigned int > connectedTo( unsigned int gid ) const;

    std::vector< vec3 > getAllPathsPoints( unsigned int gid,
                                           const std::set< unsigned int >& gidsPost,
                                           float pointSize,
                                           TNeuronConnection type = PRESYNAPTIC ) const;

    std::vector< nsol::NeuronMorphologySectionPtr >
      findPathToSoma( const nsol::MorphologySynapsePtr synapse,
                      TNeuronConnection type = PRESYNAPTIC ) const;

  protected:



    tSectionsInfoMap parseSections( const std::set< nsol::SynapsePtr >& synapses,
                                    const tSectionsMap& endSections,
                                    TNeuronConnection type = PRESYNAPTIC ) const;

    unsigned int findSynapseSegment(  const vec3& synapsePos,
                                      const nsol::Nodes& nodes ) const;

    tSectionsMap findEndSections( const std::set< nsol::SynapsePtr >& synapses,
                                  TNeuronConnection type = PRESYNAPTIC ) const;

    tFixedSynapseInfo projectSynapse( const vec3 synapsePos,
                                      const utils::PolylineInterpolation& nodes ) const;

    tFixedSynapseInfo findClosestPointToSynapse( const vec3 synapsePos,
                                                 const utils::PolylineInterpolation& nodes ) const;



    std::vector< vec3 > cutEndSection( const std::vector< vec3 >& nodes,
                                       const vec3&  synapsePos,
                                       unsigned int index ) const;

    void calculateFixedSynapseSections( void );

    std::vector< nsol::NeuronMorphologySectionPtr > getChildrenSections( nsol::MorphologySynapsePtr synapse,
                                                                         TNeuronConnection type = PRESYNAPTIC ) const;

    nsol::DataSet* _dataset;

    std::unordered_map< unsigned int , tSynapseFixedSections> _fixedSynapseSections;

  };
}



#endif /* PATHFINDER_H_ */
