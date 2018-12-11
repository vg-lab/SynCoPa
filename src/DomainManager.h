/*
 * @file  DomainManager.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_DOMAINMANAGER_H_
#define SRC_DOMAINMANAGER_H_

#include "types.h"

#include <QPolygonF>


namespace syncopa
{
  enum TMode
  {
    SYNAPSES = 0,
    PATHS,
    UNDEFINED
  };


  class DomainManager
  {
  public:

    DomainManager( );
    ~DomainManager( );

    void dataset( nsol::DataSet* dataset_ );

    void loadSynapses( unsigned int presynapticGID,
                       const gidUSet& postsynapticGIDs );

    void loadSynapses( const gidUSet& gids );

    void synapseMappingAttrib( TBrainSynapseAttribs attrib );
    void updateSynapseMapping( void );

    const TSynapseInfo& synapsesInfo( void ) const;

    void setSynapseFilteringState( bool state );
    void setSynapseFilter( float maxValue, float minValue, bool invertFilter );

    const tsynapseVec& getSynapses( void ) const;
    const tsynapseVec& getFilteredSynapses( void ) const;

    const tFloatVec& getNormValues( void ) const;
    tFloatVec getFilteredNormValues( void ) const;

    const QPolygonF& getSynapseMappingPlot( void ) const;

    gidUSet connectedTo( unsigned int gid ) const;

    std::pair< float, float > rangeBounds( void ) const;

  protected:

    void _filterSynapses( void );

    void _loadSynapseInfo( void );

    tsynapseVec _loadSynapses( const gidUSet& gids, bool log = false ) const;

    void _loadSynapses( unsigned int presynapticGID,
                        const gidUSet& postsynapticGIDs );

    void _calculateSynapsesAttribValues( const tsynapseVec& synapses );

    void _generateHistogram( const std::vector< float >& values,
                             float minValue, float maxValue );

    nsol::DataSet* _dataset;

    tsynapseVec _synapses;

    TSynapseInfo _synapseFixInfo;

    unsigned int _presynapticGID;

    TBrainSynapseAttribs _currentAttrib;

    // Histogram attributes
    std::unordered_map< unsigned int, unsigned int > _synapseIDToValues;
    std::vector< float > _currentAttribValues;
    std::vector< float > _currentAttribNormValues;
    float _maxValue;
    float _minValue;

    unsigned int _binsNumber;
    std::vector< unsigned int > _synapseAttribHistogram;
    QPolygonF _histoFunction;

    // Filter attributes
    bool _filtering;
    float _filterMinValue;
    float _filterMaxValue;
    bool _filterInvert;

    tsynapseVec _filteredSynapses;
    std::unordered_set< unsigned int > _removedSynapses;

  };


}



#endif /* SRC_DOMAINMANAGER_H_ */
