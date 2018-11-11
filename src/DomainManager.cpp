/*
 * @file  DomainManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#include "DomainManager.h"

namespace syncopa
{

  DomainManager::DomainManager( )
  : _dataset( nullptr )
  , _presynapticGID( 0 )
  , _currentAttrib( TBSA_SYNAPSE_OTHER )
  , _maxValue( 0 )
  , _minValue( 0 )
  , _binsNumber( 20 )
  , _filtering( false )
  , _filterMinValue( std::numeric_limits< float >::min( ) )
  , _filterMaxValue( std::numeric_limits< float >::max( ))
  , _filterInvert( false )
  { }

  void DomainManager::dataset( nsol::DataSet* dataset_ )
  {
    _dataset = dataset_;

    _loadSynapseInfo( );
  }

  void DomainManager::_loadSynapseInfo( void )
  {
    if( _dataset )
    {
      auto blueConfig = _dataset->blueConfig( );

      brain::Circuit brainCircuit( *blueConfig );
      brion::GIDSet gidSetBrain = brainCircuit.getGIDs( _dataset->blueConfigTarget( ));

      const brain::Synapses& brainSynapses =
         brainCircuit.getAfferentSynapses( gidSetBrain,
                                           brain::SynapsePrefetch::attributes );

      auto synapses = _dataset->circuit( ).synapses( );
      for( unsigned int i = 0; i < synapses.size( ); ++i )
      {
        auto synapse = dynamic_cast< nsolMSynapse_ptr >( synapses[ i ]);
        auto brainSynapse = brainSynapses[ synapse->gid( ) - 1 ];

        tBrainSynapse infoPre =
           std::make_tuple( brainSynapse.getPresynapticSectionID( ),
                            brainSynapse.getPresynapticSegmentID( ),
                            brainSynapse.getPresynapticDistance( ));

        tBrainSynapse infoPost =
           std::make_tuple( brainSynapse.getPostsynapticSectionID( ),
                            brainSynapse.getPostsynapticSegmentID( ),
                            brainSynapse.getPostsynapticDistance( ));

        tBrainSynapseAttribs attribs =
           std::make_tuple( brainSynapse.getDelay( ),
                            brainSynapse.getConductance( ),
                            brainSynapse.getUtilization( ),
                            brainSynapse.getDepression( ),
                            brainSynapse.getFacilitation( ),
                            brainSynapse.getDecay( ),
                            brainSynapse.getEfficacy( ));

        _synapseFixInfo.insert(
           std::make_pair( synapse, std::make_tuple( attribs, infoPre, infoPost )));

    }

     std::cout << "Loaded BRAIN synapse info of " << _synapseFixInfo.size( ) << std::endl;
     //      calculateFixedSynapseSections( );
   }
  }

  const std::vector< nsolMSynapse_ptr >& DomainManager::getSynapses( void ) const
  {
    return _synapses;
  }

  void DomainManager::loadSynapses( unsigned int presynapticGID,
                                    const gidUSet& postsynapticGIDs )
  {
    _loadSynapses( presynapticGID, postsynapticGIDs );
  }

  void DomainManager::loadSynapses( const gidUSet& gids )
  {
    if( gids.empty( ))
      return;

    _synapses = _loadSynapses( gids );
    _filteredSynapses = _synapses;

    std::cout << "Loaded " << _synapses.size( ) << " synapses." << std::endl;

  }

  tsynapseVec DomainManager::_loadSynapses( const gidUSet& gids, bool log ) const
  {
    tsynapseVec result;

    std::set< unsigned int > gidset( gids.begin( ), gids.end( ));

     auto& circuit = _dataset->circuit( );
     auto synapseSet = circuit.synapses( gidset,
                                         nsol::Circuit::PRESYNAPTICCONNECTIONS );

     result.reserve( synapseSet.size( ));

     for( auto syn : synapseSet )
     {

       auto msyn = dynamic_cast< nsolMSynapse_ptr >( syn );
       if( !msyn)
       {
         if( log )
           std::cout << "EMPTY SYNAPSE " << std::endl;
         continue;
       }

       auto sectionPre = msyn->preSynapticSection( );
       auto sectionPost = msyn->postSynapticSection( );

       // axo-somatic
       if( !sectionPre || ( !sectionPost &&
           msyn->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC ))
         continue;


       auto synInfo = _synapseFixInfo.find( msyn );
       if( synInfo == _synapseFixInfo.end( ))
         continue;

       auto infoPre = std::get< TBSI_PRESYNAPTIC >( synInfo->second );
       auto infoPost = std::get< TBSI_POSTSYNAPTIC >( synInfo->second );

       if( std::get< TBS_SEGMENT_INDEX >( infoPre ) >=
           sectionPre->nodes( ).size( ) - 1 )
       {
         if( log )
         std::cout << "Discarding synapse with pre segment "
                   << std::get< TBS_SEGMENT_INDEX >( infoPre )
                   << " segments " << ( sectionPre->nodes( ).size( ) - 1 )
                   << std::endl;
         continue;
       }

       if( sectionPost && std::get< TBS_SEGMENT_INDEX >( infoPost ) >=
           sectionPost->nodes( ).size( ) - 1 )
       {
         if( log )
         std::cout << "Discarding synapse with post segment "
                   << std::get< TBS_SEGMENT_INDEX >( infoPost )
                   << " segments " << ( sectionPost->nodes( ).size( ) - 1 )
                   << std::endl;
         continue;
       }

       result.push_back( msyn );
     }

     result.shrink_to_fit( );

     return result;
  }

  void DomainManager::_loadSynapses( unsigned int presynapticGID,
                                  const gidUSet& postsynapticGIDs )
  {
    if( presynapticGID == _presynapticGID && postsynapticGIDs.empty( ))
      return;

    std::vector< nsolMSynapse_ptr > result;

    std::set< unsigned int > gidsPre = { presynapticGID };

    auto& circuit = _dataset->circuit( );
    auto synapseSet = circuit.synapses( gidsPre,
                                        nsol::Circuit::PRESYNAPTICCONNECTIONS );


    result.reserve( synapseSet.size( ));

    for( auto syn : synapseSet )
    {
      if( !postsynapticGIDs.empty( ) &&
          postsynapticGIDs.find( syn->postSynapticNeuron( )) == postsynapticGIDs.end( ))
        continue;


      auto msyn = dynamic_cast< nsolMSynapse_ptr >( syn );
      if( !msyn)
      {
        std::cout << "EMPTY SYNAPSE " << std::endl;
        continue;
      }

      auto sectionPre = msyn->preSynapticSection( );
      auto sectionPost = msyn->postSynapticSection( );

      // axo-somatic
      if( !sectionPre || ( !sectionPost &&
          msyn->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC ))
        continue;


      auto synInfo = _synapseFixInfo.find( msyn );
      if( synInfo == _synapseFixInfo.end( ))
        continue;

      auto infoPre = std::get< TBSI_PRESYNAPTIC >( synInfo->second );
      auto infoPost = std::get< TBSI_POSTSYNAPTIC >( synInfo->second );

      if( std::get< TBS_SEGMENT_INDEX >( infoPre ) >=
          sectionPre->nodes( ).size( ) - 1 )
      {
        std::cout << "Discarding synapse with pre segment "
                  << std::get< TBS_SEGMENT_INDEX >( infoPre )
                  << " segments " << ( sectionPre->nodes( ).size( ) - 1 )
                  << std::endl;
        continue;
      }

      if( sectionPost && std::get< TBS_SEGMENT_INDEX >( infoPost ) >=
          sectionPost->nodes( ).size( ) - 1 )
      {
        std::cout << "Discarding synapse with post segment "
                  << std::get< TBS_SEGMENT_INDEX >( infoPost )
                  << " segments " << ( sectionPost->nodes( ).size( ) - 1 )
                  << std::endl;
        continue;
      }

      result.push_back( msyn );
    }

    result.shrink_to_fit( );

    _synapses = result;
    _filteredSynapses = _synapses;

    std::cout << "Loaded " << _synapses.size( ) << " synapses." << std::endl;

  }

  void DomainManager::synapseMappingAttrib( TBrainSynapseAttribs attrib )
  {
    _currentAttrib = attrib;

    updateSynapseMapping( );
  }

  void DomainManager::updateSynapseMapping( void )
  {
    _calculateSynapsesAttribValues( _synapses );

    _generateHistogram( _currentAttribValues, _minValue, _maxValue );
  }

  void DomainManager::setSynapseFilteringState( bool state )
  {
    std::cout << "Established filtering to " << std::boolalpha << state << std::endl;
    _filtering = state;
  }

  void DomainManager::setSynapseFilter( float minValue, float maxValue,
                                        bool invertFilter )
  {
    double rangeFactor = ( _maxValue - _minValue );

    _filterMaxValue = ( maxValue * rangeFactor ) + _minValue;
    _filterMinValue = ( minValue * rangeFactor ) + _minValue;
    _filterInvert = invertFilter;

    std::cout << "Filtering range: " << _filterMinValue
              << " " << _filterMaxValue << std::endl;

    _filterSynapses( );
  }

  void DomainManager::_filterSynapses( void )
  {
    _removedSynapses.clear( );

    _filteredSynapses.clear( );
    _filteredSynapses.reserve( _synapses.size( ));

    for( auto synapse : _synapses )
    {
      auto idx = _synapseIDToValues.find( synapse->gid( ));
      assert( idx != _synapseIDToValues.end( ));

      float value = _currentAttribValues[ idx->second ];

      if( value < _minValue || value > _maxValue )
        std::cout << "Synapse attrib " << ( unsigned int )_currentAttrib
                  << " value out of bounds " << _minValue
                  << " - " << _maxValue
                  << " value " << value
                  << std::endl;

      if( !_filterInvert )
      {
        if( value >= _filterMinValue && value <= _filterMaxValue )
          _filteredSynapses.push_back( synapse );
        else
          _removedSynapses.insert( synapse->gid( ));

      }
      else
      {
        if( value <= _filterMinValue || value >= _filterMaxValue )
          _filteredSynapses.push_back( synapse );
        else
          _removedSynapses.insert( synapse->gid( ));
      }
    }

    _filteredSynapses.shrink_to_fit( );

    std::cout << "Filtered synapses " << _filteredSynapses.size( )
              << " out of " << _synapses.size( )
              << std::endl;

  }

  const tsynapseVec& DomainManager::getFilteredSynapses(  ) const
  {
    if( !_filtering )
      return _synapses;
    else
      return _filteredSynapses;
  }

  const tFloatVec& DomainManager::getNormValues( void ) const
  {
    return _currentAttribNormValues;
  }

  tFloatVec DomainManager::getFilteredNormValues( void ) const
  {
    if( !_filtering )
      return _currentAttribNormValues;

    tFloatVec result;
    result.reserve( _currentAttribNormValues.size( ));

    for( unsigned int i = 0; i < _currentAttribNormValues.size( ); ++i )
    {
      auto synapse = _synapses[ i ];
      if( _removedSynapses.find( synapse->gid( )) == _removedSynapses.end( ))
        result.push_back( _currentAttribNormValues[ i ]);
    }

    result.shrink_to_fit( );

    return result;
  }


  const TSynapseInfo& DomainManager::synapsesInfo( void ) const
  {
    return _synapseFixInfo;
  }

  void DomainManager::_calculateSynapsesAttribValues( const tsynapseVec& synapses )
  {
    auto& synapseInfoMap = _synapseFixInfo;

    std::vector< float > values;
    values.reserve( synapses.size( ));

    float maxValue = 0.0f;
    float minValue = std::numeric_limits< float >::max( );

    _synapseIDToValues.clear( );

    unsigned int counter = 0;
    for( auto synapse : synapses )
 //    for( unsigned int i = 0; i < synapses.size( ); ++i )
    {
 //      auto synapse = synapses[ i ];

      auto synapseInfo = synapseInfoMap.find( synapse );
      assert( synapseInfo != synapseInfoMap.end( ));
//      if( synapseInfo == synapseInfoMap.end( ))
//      {
//        std::cout << "ERROR: " << i << " synapse info " << synapse->gid( ) << " not found." << std::endl;
//        continue;
//      }


      float value = 0.0f;
      auto synapseAttribs = std::get< TBSI_ATTRIBUTES >( synapseInfo->second );

      value = getSynapseAttribValue( synapse, synapseAttribs, _currentAttrib );

      maxValue = std::max( maxValue, value );
      minValue = std::min( minValue, value );

      values.emplace_back( value );

      _synapseIDToValues.insert( std::make_pair( synapse->gid( ), counter ));

      ++counter;
    }

    _currentAttribValues = values;

    _currentAttribNormValues.clear( );
    _currentAttribNormValues.resize( values.size( ), 0.0f );

    _minValue = minValue;
    _maxValue = maxValue;

    double invRange = 1.0 / ( maxValue - minValue );

    for( unsigned int i = 0; i < values.size( ); ++i )
    {
      float normalizedValue = ( values[ i ] - minValue ) * invRange;
      normalizedValue = std::min( std::max( 0.0f, normalizedValue ), 1.0f );
//      _gidToParticleId.insert( std::make_pair( synapses[ i ]->gid( ),
//                                               particle.id( )));
      _currentAttribNormValues[ i ] = normalizedValue;
    }
  }


  void DomainManager::_generateHistogram( const std::vector< float >& values,
                                      float minValue, float maxValue )
  {
    assert( _binsNumber > 0 );

    std::cout << "Total values: " << values.size( ) << std::endl;

    _synapseAttribHistogram.clear( );
    if( _synapseAttribHistogram.size( ) != _binsNumber )
      _synapseAttribHistogram.resize( _binsNumber, 0 );

    _histoFunction.clear( );

    float invNormValues = 1.0f / ( maxValue - minValue );

    unsigned int maxBin = 0;
    unsigned int minBin = std::numeric_limits< unsigned int >::max( );

    for( auto value : values )
    {
      unsigned int pos = (( value - minValue ) * invNormValues ) * ( _binsNumber - 1 );

      unsigned int& bin = _synapseAttribHistogram[ pos ];
      bin += 1;
    }

    std::cout << "Bins: ";
    for( auto bin : _synapseAttribHistogram )
    {
      std::cout << " " << bin;

      if( bin > maxBin )
        maxBin = bin;

      if( bin < minBin )
        minBin = bin;

    }
    std::cout << std::endl;

    std::cout << "Max bin " << maxBin << " min bin " << minBin << std::endl;

    float step = 1.0f / ( _binsNumber - 1 );
    float acc = 0.0f;

    double invNormHistogram = 1.0 / ( maxBin - minBin );

    std::cout << "Histogram: ";
    for( auto bin : _synapseAttribHistogram )
    {
      double normalizedY = ( bin - minBin ) * invNormHistogram;
      double normalizedX = acc;

      _histoFunction.append( QPointF( normalizedX, normalizedY ));

      std::cout << " (" << normalizedX << ", " << normalizedY << ")";

      acc += step;
    }
    std::cout << std::endl;

  }

  const QPolygonF& DomainManager::getSynapseMappingPlot( void ) const
  {
    return _histoFunction;
  }


  gidUSet DomainManager::connectedTo( unsigned int gid ) const
  {
    gidUSet result = { gid };

    auto synapses = _loadSynapses( result );

    for( auto syn : synapses )
    {
      if( syn->preSynapticNeuron( ) != gid )
        continue;

      result.insert( syn->postSynapticNeuron( ));
    }

    return result;
  }


}

