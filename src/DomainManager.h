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


#include <nlrender/nlrender.h>
#include <prefr/prefr.h>
#include <nsol/nsol.h>

#include "NeuronScene.h"
#include "PSManager.h"
#include "PathFinder.h"
#include "DynamicPathManager.h"

#include "types.h"

namespace syncopa
{

  class DomainManager
  {

  public:

    DomainManager( );
    ~DomainManager( );

    const nsol::DataSet* dataSet( void ) const;

    void nlRenderer( nlrender::Renderer* renderer );

    void init( void );
    void clear( void );
    void clearSelection( void );

    void createParticleSystem( void );

    void loadBlueConfig( const std::string& blueConfigFilePath,
                         const std::string& target );

    void loadSynapses( const gidUSet& gids );
    const tsynapseVec& getSynapses( void ) const;

    void renderMorphologies( void );
    void processDynamicEvents( void );

    void configureSynapses( const tsynapseVec& synapses,
                            bool mapValues = false,
                            TBrainSynapseAttribs attrib = TBSA_SYNAPSE_OTHER,
                            TNeuronConnection type = ALL_CONNECTIONS );

    prefr::ParticleSystem* particleSystem( void ) const;
    const TSynapseInfo& synapseFixInfo( void ) const;
    float particleSizeThreshold( void );

    const QPolygonF& getSynapseMappingPlot( void ) const;

    nlgeometry::AxisAlignedBoundingBox boundingBox( void ) const;

  public slots:

    void selectPresynapticNeuron( unsigned int gid );
    void selectPostsynapticNeuron( const std::vector< unsigned int >& gid );

    const std::vector< nsol::MorphologySynapsePtr >& currentSynapses( void );

    void colorSelectedPre( const syncopa::vec3& color );
    void colorSelectedPost( const syncopa::vec3& color );
    void colorRelated( const syncopa::vec3& color );
    void colorContext( const syncopa::vec3& color );
    void colorSynapsesPre( const syncopa::vec3& color );
    void colorSynapsesPost( const syncopa::vec3& color );
    void colorPathsPre( const syncopa::vec3& color );
    void colorPathsPost( const syncopa::vec3& color );

    void colorSynapseMapPre( const syncopa::vec3& color );
    void colorSynapseMapPost( const syncopa::vec3& color );
    void colorSynapseMap( const tQColorVec& colors );


    const syncopa::vec3& colorSelectedPre( void ) const;
    const syncopa::vec3& colorSelectedPost( void ) const;
    const syncopa::vec3& colorRelated( void ) const;
    const syncopa::vec3& colorContext( void ) const;
    const syncopa::vec3& colorSynapsesPre( void ) const;
    const syncopa::vec3& colorSynapsesPost( void ) const;
    const syncopa::vec3& colorPathsPre( void ) const;
    const syncopa::vec3& colorPathsPost( void ) const;

    const syncopa::vec3& colorSynapseMapPre( void ) const;
    std::vector< std::pair< float, QColor >> colorSynapseMap( void ) const;

    void showSelectedPre( int state );
    void showSelectedPost( int state );
    void showRelated( int state );
    void showContext( int state );
    void showSynapsesPre( int state );
    void showSynapsesPost( int state );
    void showPathsPre( int state );
    void showPathsPost( int state );

    void setSynapseMapping( int attrib = ( int ) TBSA_SYNAPSE_OTHER );
    void setSynapseMappingState( bool state );

    void startDynamic( void );
    void stopDynamic( void );

    void updatePathsVisibility( void );
    void updateSynapsesVisibility( void );
    void updateBoundingBox( void );

  protected:

    void setupNeuronMorphologies( void );

    void setupSynapses( void );

    void setupPaths( const gidUSet& gidsPre,
                     const gidUSet& gidsPost );

    void _loadSynapses( unsigned int presynapticGID,
                        const gidUSet& postsynapticGIDs );


    void _loadSynapseFixInfo( void );

    void _calculateSynapsesAttribValues( const tsynapseVec& synapses );

    void _generateHistogram( const std::vector< float >& values,
                             float minValue, float maxValue );

    nsol::DataSet* _dataset;
    syncopa::NeuronScene* _neuronScene;
    syncopa::PSManager* _psManager;
    syncopa::PathFinder* _pathFinder;
    syncopa::DynamicPathManager* _dynPathManager;
    nlrender::Renderer* _nlrenderer;

    tsynapseVec _synapses;

    TSynapseInfo _synapseFixInfo;


    float _particleSizeThreshold;

    syncopa::TRenderMorpho _neuronsSelectedPre;
    syncopa::TRenderMorpho _neuronsSelectedPost;
    syncopa::TRenderMorpho _neuronsRelated;
    syncopa::TRenderMorpho _neuronsContext;

    gidUSet _gidsAll;
    gidUSet _gidsSelectedPre;
    gidUSet _gidsSelectedPost;
    gidUSet _gidsRelated;
    gidUSet _gidsOther;

    syncopa::vec3 _colorSelectedPre;
    syncopa::vec3 _colorSelectedPost;
    syncopa::vec3 _colorRelated;
    syncopa::vec3 _colorContext;
    syncopa::vec3 _colorSynapsesPre;
    syncopa::vec3 _colorSynapsesPost;
    syncopa::vec3 _colorPathsPre;
    syncopa::vec3 _colorPathsPost;

    syncopa::vec3 _colorSynMapPre;
    syncopa::vec3 _colorSynMapPost;

    tQColorVec _colorSynMap;

    float _alphaSynapsesPre;
    float _alphaSynapsesPost;
    float _alphaPathsPre;
    float _alphaPathsPost;

    float _alphaSynapseMapPre;

    bool _showSelectedPre;
    bool _showSelectedPost;
    bool _showRelated;
    bool _showContext;
    bool _showSynapsesPre;
    bool _showSynapsesPost;
    bool _showPathsPre;
    bool _showPathsPost;

    std::vector< nsol::MorphologySynapsePtr > _currentSynapses;

    bool _mapSynapseValues;
    TBrainSynapseAttribs _currentSynapseAttrib;

    std::vector< float > _currentAttribValues;
    std::vector< float > _currentAttribNormValues;
    float _maxValue;
    float _minValue;

    unsigned int _binsNumber;
    std::vector< unsigned int > _synapseAttribHistogram;
    QPolygonF _histoFunction;

    nlgeometry::AxisAlignedBoundingBox _boundingBox;
  };


}



#endif /* SRC_DOMAINMANAGER_H_ */
