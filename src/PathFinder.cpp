/*
 * @file	PathFinder.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "PathFinder.h"

#include <brain/brain.h>
#include <QDebug>

namespace syncopa
{

  PathFinder::PathFinder( void )
    : _dataset( nullptr )
    , _synapseFixInfo( nullptr )
    , _maxDepth( 0 )
  { }

  PathFinder::~PathFinder( void )
  {

  }

  void PathFinder::dataset( nsol::DataSet* dataset_ ,
                            const TSynapseInfo* synapseInfo )
  {
    _dataset = dataset_;
    _synapseFixInfo = synapseInfo;
  }

  void PathFinder::configure(
    const std::vector< nsol::SynapsePtr >& synapses ,
    const std::unordered_set< unsigned int >& preNeuronsWithAllPaths ,
    const std::unordered_set< unsigned int >& postNeuronsWithAllPaths ,
    const std::unordered_set< unsigned int >& preNeuronsWithConnectedPaths ,
    const std::unordered_set< unsigned int >& postNeuronsWithConnectedPaths ,
    float pointSize ,
    std::vector< vec3 >& preOut ,
    std::vector< vec3 >& postOut )
  {
    clear( );

    tsynapseVec outUsedSynapses;
    tsynapseVec outUsedPreSynapses;
    tsynapseVec outUsedPostSynapses;

    _calculateSynapses(
      synapses ,
      preNeuronsWithAllPaths ,
      postNeuronsWithAllPaths ,
      preNeuronsWithConnectedPaths ,
      postNeuronsWithConnectedPaths ,
      outUsedSynapses ,
      outUsedPreSynapses ,
      outUsedPostSynapses
    );

    _populateTrees( outUsedPreSynapses , outUsedPostSynapses );

    _processSections( outUsedPreSynapses , outUsedPostSynapses );
    _processEndSections( outUsedPreSynapses , outUsedPostSynapses );


    std::unordered_set< nsol::NeuronMorphologySectionPtr > insertedSections;
    for ( const auto& synapse: outUsedPreSynapses )
    {
      _createPath( insertedSections , preOut , synapse , PRESYNAPTIC ,
                   pointSize );
    }

    for ( const auto& synapse: outUsedPostSynapses )
    {
      _createPath( insertedSections , postOut , synapse , POSTSYNAPTIC ,
                   pointSize );
    }
  }

  void PathFinder::_populateTrees( const tsynapseVec& preSynapses ,
                                   const tsynapseVec& postSynapses )
  {

    for ( auto syn: preSynapses )
    {
      auto sectionPre = syn->preSynapticSection( );
      if ( sectionPre )
      {
        auto treePre = _treePre.find( syn->preSynapticNeuron( ));
        if ( treePre == _treePre.end( ))
        {
          auto tree = ConnectivityTree( );
          treePre =
            _treePre.insert(
              std::make_pair( syn->preSynapticNeuron( ) , tree )).first;

        }

        if ( !treePre->second.hasNode( sectionPre ))
        {
          auto sections = pathToSoma( sectionPre );
          treePre->second.addBranch( sections );
        }
      }
      else
      {
        std::cout << "ERROR: Unassigned synapse pre section " << syn->gid( )
                  << " " << syn->preSynapticNeuron( )
                  << "-" << syn->postSynapticNeuron( )
                  << std::endl;
      }
    }

    for ( auto syn: postSynapses )
    {
      auto sectionPost = syn->postSynapticSection( );
      if ( sectionPost ||
           syn->synapseType( ) == nsol::MorphologySynapse::AXOSOMATIC )
      {
        auto treePost = _treePost.find( syn->postSynapticNeuron( ));
        if ( treePost == _treePost.end( ))
        {
          auto tree = ConnectivityTree( );
          treePost =
            _treePost.insert(
              std::make_pair( syn->postSynapticNeuron( ) , tree )).first;

        }

        if ( !treePost->second.hasNode( sectionPost ))
        {
          auto postSections = pathToSoma( sectionPost );

          treePost->second.addBranch( postSections );
        }
      }
      else
      {
        std::cout << "ERROR: Unassigned synapse post section " << syn->gid( )
                  << " " << syn->preSynapticNeuron( )
                  << "-" << syn->postSynapticNeuron( )
                  << std::endl;
      }
    }
  }

  const std::unordered_map< unsigned int , ConnectivityTree >&
  PathFinder::presynapticTrees( void ) const
  {
    return _treePre;
  }


  void PathFinder::clear( void )
  {

    _computedPaths.clear( );
    _treePre.clear( );
    _treePost.clear( );

    _pathsPre.clear( );
    _pathsPost.clear( );

    _infoSections.clear( );

    _maxDepth = 0;
  }


  void PathFinder::_calculateSynapses(
    const std::vector< nsol::SynapsePtr >& synapses ,
    const std::unordered_set< unsigned int >& preNeuronsWithAllPaths ,
    const std::unordered_set< unsigned int >& postNeuronsWithAllPaths ,
    const std::unordered_set< unsigned int >& preNeuronsWithConnectedPaths ,
    const std::unordered_set< unsigned int >& postNeuronsWithConnectedPaths ,
    tsynapseVec& outUsedSynapses ,
    tsynapseVec& outUsedPreSynapses ,
    tsynapseVec& outUsedPostSynapses ) const
  {

    auto contains = [ ]( const std::unordered_set< unsigned int >& set ,
                         unsigned int value )
    {
      return set.find( value ) != set.cend( );
    };

    for ( auto syn: synapses )
    {
      auto morphSyn = dynamic_cast<nsol::MorphologySynapse*>(syn);
      if ( morphSyn == nullptr ) continue;

      auto pre = syn->preSynapticNeuron( );
      auto post = syn->postSynapticNeuron( );
      bool add = false;

      // Pre-synaptic paths
      if ( contains( preNeuronsWithAllPaths , pre )
           || ( contains( preNeuronsWithConnectedPaths , pre )
                && ( contains( postNeuronsWithAllPaths , post )
                     || contains( postNeuronsWithConnectedPaths , post ))))
      {
        add = true;
        outUsedPreSynapses.push_back( morphSyn );
      }

      // Post-synaptic paths
      if ( contains( postNeuronsWithAllPaths , post )
           || ( contains( postNeuronsWithConnectedPaths , post )
                && ( contains( preNeuronsWithAllPaths , pre )
                     || contains( preNeuronsWithConnectedPaths , pre ))))
      {
        add = true;
        outUsedPostSynapses.push_back( morphSyn );
      }

      if ( add )
      {
        outUsedSynapses.push_back( morphSyn );
      }
    } // for synapses
  }

  void PathFinder::_createPath(
    std::unordered_set< nsol::NeuronMorphologySectionPtr >& insertedSections ,
    std::vector< vec3 >& result ,
    nsol::MorphologySynapse* syn ,
    TNeuronConnection type ,
    float pointSize ) const
  {
    auto currentGid = type == PRESYNAPTIC ? syn->preSynapticNeuron( )
                                          : syn->postSynapticNeuron( );
    auto transform = getTransform( currentGid );
    auto sectionNodes = pathToSoma( syn , type );
    for ( auto section: sectionNodes )
    {
      if ( insertedSections.find( section ) != insertedSections.end( ))
        continue;
      insertedSections.insert( section );

      nsol::Nodes& nodes = section->nodes( );
      utils::PolylineInterpolation pathPoints;
      auto parsedSection = _infoSections.find( section );
      if ( parsedSection != _infoSections.end( ) &&
           std::get< tsi_leafSection >( parsedSection->second ))
      {
        auto points = std::get< tsi_fixedSection >( parsedSection->second );
        pathPoints.insert( points );
      }
      else
      {
        for ( auto node: nodes )
          pathPoints.insert( transformPoint( node->point( ) , transform ));
      }

      float currentDist = 0;
      float accDist = pathPoints.totalDistance( );
      while ( currentDist < accDist )
      {
        vec3 pos = pathPoints.pointAtDistance( currentDist );
        result.push_back( pos );

        currentDist += pointSize;
      } // while
    } // for section
  }


  std::vector< nsol::NeuronMorphologySectionPtr >
  PathFinder::pathToSoma( const nsolMSynapse_ptr synapse ,
                          syncopa::TNeuronConnection type ) const
  {
    auto* section =
      type == PRESYNAPTIC
      ? dynamic_cast< nsolMSection_ptr >( synapse->preSynapticSection( ))
      : dynamic_cast< nsolMSection_ptr >( synapse->postSynapticSection( ));

    return pathToSoma( section );
  }

  std::vector< nsolMSection_ptr >
  PathFinder::pathToSoma( nsolMSection_ptr section ) const
  {
    std::vector< nsol::NeuronMorphologySection* > sections;
    while ( section != nullptr )
    {
      sections.push_back( section );
      section = dynamic_cast< nsol::NeuronMorphologySection* >(
        section->parent( )
      );
    }

    return sections;
  }

  mat4 PathFinder::getTransform( unsigned int gid ) const
  {
    nsol::NeuronsMap& neurons = _dataset->neurons( );
    auto neuron = neurons.find( gid );
    if ( neuron == neurons.end( ))
      return mat4::Ones( );

    return neuron->second->transform( );
  }


  std::vector< vec3 >
  PathFinder::_cutEndSection( const std::vector< vec3 >& nodes ,
                              const vec3& synapsePos ,
                              unsigned int lastIndex ) const
  {
    std::vector< vec3 > result;
    for ( unsigned int i = 0; i < lastIndex + 1; ++i )
    {
      result.push_back( nodes[ i ] );
    }
    result.push_back( synapsePos );

    return result;
  }

  void PathFinder::_processSections(
    const std::vector< nsolMSynapse_ptr >& preSynapses ,
    const std::vector< nsolMSynapse_ptr >& postSynapses )
  {
    _infoSections.clear( );
    _somaSynapses.clear( );

    auto lambda = [ & ](
      const nsol::MorphologySynapsePtr synapse ,
      unsigned int neuronGID ,
      const mat4& transform ,
      const nsol::NeuronMorphologySectionPtr section ,
      const tBrainSynapse& synapseFixInfo )
    {
      if ( std::get< TBS_SECTION_ID >( synapseFixInfo ) != section->id( ))
      {
        std::cout << "ERROR: Section " << section->id( )
                  << " does not match "
                  << std::get< TBS_SECTION_ID >( synapseFixInfo )
                  << std::endl;
        return;
      }

      unsigned int segmentIndex = std::get< TBS_SEGMENT_INDEX >(
        synapseFixInfo );
      const auto& nodes = section->nodes( );

      if ( nodes.empty( ) || nodes.size( ) == 1 )
      {
        std::cout << "ERROR: Section " << section->id( )
                  << " with " << nodes.size( )
                  << " nodes." << std::endl;
        return;
      }

      if ( segmentIndex >= nodes.size( ) - 1 )
      {
        std::cout << "ERROR: " << neuronGID << " Segment index "
                  << segmentIndex
                  << " is greater than section " << section->id( )
                  << " nodes number " << nodes.size( )
                  << " distance "
                  << std::get< TBS_SEGMENT_DISTANCE >( synapseFixInfo )
                  << std::endl;
        return;
      }

      float distance = std::get< TBS_SEGMENT_DISTANCE >( synapseFixInfo );
      vec3 start = nodes[ segmentIndex ]->point( );
      vec3 end = nodes[ segmentIndex + 1 ]->point( );

      float length = ( end - start ).norm( );

      float normalized = distance / length;

      vec3 synapsePos = transformPoint(
        end * normalized + start * ( 1 - normalized ) , transform );

      auto it = _infoSections.find( section );
      if ( it == _infoSections.end( ))
      {
        utils::PolylineInterpolation interpolator;

        for ( auto node: section->nodes( ))
          interpolator.insert( transformPoint( node->point( ) , transform ));

        std::unordered_map< nsol::MorphologySynapsePtr ,
          tFixedSynapseInfo > synapseSet;

        float sectionDist = interpolator.totalDistance( );

        std::vector< vec3 > fixedSection;

        tSectionInfo data =
          std::make_tuple( synapseSet , sectionDist ,
                           false ,
                           fixedSection , interpolator );

        it = _infoSections.insert( std::make_pair( section , data )).first;

      } // if section not found

      auto& synapseMap = std::get< tsi_Synapses >( it->second );
      auto& interpolator = std::get< tsi_Interpolator >( it->second );
      float composedDistance = interpolator.distance( segmentIndex );

      composedDistance += distance;

      tFixedSynapseInfo fixedSynInfo =
        std::make_tuple( synapsePos , composedDistance , 0 , segmentIndex ,
                         true );

      synapseMap.insert( std::make_pair( synapse , fixedSynInfo ));
    };

    for ( const auto& synapse: preSynapses )
    {
      if ( !synapse->preSynapticSection( ))
      {
        std::cout << "ERROR: Synapse " << synapse->gid( )
                  << " section NOT FOUND" << std::endl;
        continue;
      }

      auto synInfo = _synapseFixInfo->find( synapse );
      if ( synInfo == _synapseFixInfo->end( ))
      {
        std::cout << "ERROR: Synapse " << synapse->gid( ) << " info NOT FOUND"
                  << std::endl;
        continue;
      }

      auto neuronGid = synapse->preSynapticNeuron( );
      auto transform = getTransform( neuronGid );
      auto section = synapse->preSynapticSection( );
      const auto& fixInfo = std::get< TBSI_PRESYNAPTIC >( synInfo->second );

      lambda( synapse , neuronGid , transform , section , fixInfo );
    }

    for ( const auto& synapse: postSynapses )
    {
      if ( !synapse->postSynapticSection( ) &&
           synapse->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC )
      {
        std::cout << "ERROR: Synapse " << synapse->gid( )
                  << " section NOT FOUND" << std::endl;
        continue;
      }

      auto synInfo = _synapseFixInfo->find( synapse );
      if ( synInfo == _synapseFixInfo->end( ))
      {
        std::cout << "ERROR: Synapse " << synapse->gid( ) << " info NOT FOUND"
                  << std::endl;
        continue;
      }

      auto neuronGid = synapse->postSynapticNeuron( );
      auto transform = getTransform( neuronGid );
      auto section = synapse->postSynapticSection( );
      const auto& fixInfo = std::get< TBSI_POSTSYNAPTIC >( synInfo->second );

      if ( !section &&
           synapse->synapseType( ) == nsol::MorphologySynapse::AXOSOMATIC )
      {
        _somaSynapses.insert( synapse );
        std::cout << "Found somatic synapse " << synapse->gid( ) << std::endl;
        continue;
      }

      lambda( synapse , neuronGid , transform , section , fixInfo );
    }
  }

  void PathFinder::_processEndSections(
    const std::vector< nsolMSynapse_ptr >& preSynapses ,
    const std::vector< nsolMSynapse_ptr >& postSynapses )
  {

    auto lambda = [ & ]( const unsigned int gid, ConnectivityTree* tree )
    {
      auto endSections = tree->leafNodes( );
      if ( endSections.empty( ))
        std::cerr << "ERROR: " << gid << " leaf nodes ARE EMPTY!" << std::endl;

      for ( auto node: endSections )
      {
        auto section = node->section( );

        auto it = _infoSections.find( section );
        if ( it == _infoSections.end( ))
        {
          std::cerr << "Section " << section->id( ) << " not parsed correctly."
                    << std::endl;
          continue;
        }

        tSectionInfo& data = it->second;

        float maxDistance = 0;

        const auto sectionSynapses = std::get< tsi_Synapses >( data );

        if ( sectionSynapses.empty( ))
          continue;

        // Calculate farthest synapse distance on the section
        auto syn = sectionSynapses.cbegin( );
        auto farthestSynapse = syn;
        for ( ; syn != sectionSynapses.cend( ); ++syn )
        {
          float synapseDistanceOnSection =
            std::get< tpi_distance >( syn->second );

          if ( synapseDistanceOnSection > maxDistance )
          {
            maxDistance = synapseDistanceOnSection;
            farthestSynapse = syn;
          }
        }

        const tFixedSynapseInfo synapseInfo = farthestSynapse->second;

        const vec3 synapsePos = std::get< tpi_position >( synapseInfo );
        const unsigned int index = std::get< tpi_index >( synapseInfo );

        const std::vector< vec3 > fixedSection =
          _cutEndSection( std::get< tsi_Interpolator >( data ).positions( ) ,
                          synapsePos , index );

        std::get< tsi_fixedSection >( data ) = fixedSection;
        std::get< tsi_leafSection >( data ) = true;

      } // for each end section
    };

    for ( const auto& synapse: preSynapses )
    {
      auto neuron = synapse->preSynapticNeuron( );
      auto treeIt = _treePre.find( neuron );
      if ( treeIt == _treePre.end( ))
      {
        std::cerr << "ERROR: Pre tree for neuron " << neuron << " NOT FOUND"
                  << std::endl;
        continue;
      }
      lambda( neuron , &treeIt->second );
    }

    for ( const auto& synapse: postSynapses )
    {
      auto neuron = synapse->postSynapticNeuron( );
      auto treeIt = _treePost.find( neuron );
      if ( treeIt == _treePost.end( ))
      {
        std::cerr << "ERROR: Post tree for neuron " << neuron << " NOT FOUND"
                  << std::endl;
        continue;
      }
      lambda( neuron , &treeIt->second );
    }
  }

  void PathFinder::addPostsynapticPath( nsol::MorphologySynapsePtr synapse ,
                                        const tPosVec& nodes )
  {
    utils::PolylineInterpolation interpolator( nodes );

    _pathsPost.insert( std::make_pair( synapse , interpolator ));
  }


  std::pair< unsigned int , cnode_ptr >
  PathFinder::node( unsigned int sectionID ) const
  {

    for ( const auto& item: _treePre )
    {
      auto result = item.second.node( sectionID );
      if ( result != nullptr ) return std::make_pair( item.first , result );
    }

    return std::make_pair( 0 , nullptr );
  }

  utils::EventPolylineInterpolation
  PathFinder::computeDeepestPathFrom( unsigned int sectionID )
  {
    utils::EventPolylineInterpolation interpolator;

    auto originPair = node( sectionID );
    auto origin = originPair.second;
    auto originNeuron = originPair.first;
    if ( !origin )
    {
      std::cerr << "Error: node for section " << sectionID << " not found."
                << std::endl;

      return interpolator;
    }

    auto found = _computedPaths.find( sectionID );

    if ( found != _computedPaths.end( ))
      return found->second;
    else
    {

      std::vector< ConnectivityNode* > pathSections;
      pathSections.reserve( origin->childrenMaxDepth( ) + 1 );
      pathSections.push_back( origin );

      auto firstPath = origin->deepestPath( );

      for ( auto pathSection: firstPath )
      {
        pathSections.push_back( pathSection );
      }

//      std::cout << "Found deepest path with " << pathSections.size( )
//                << " out of " << origin->childrenMaxDepth( )
//                << std::endl;

      auto transform = getTransform( originNeuron );

      cnode_ptr last = nullptr;
      for ( auto currentSection: pathSections )
      {
        //        std::cout << " " << connectivityNode->section( )->id( )
        //                  << ":" << interpolator.totalDistance( )
        //                  << "->" << ( connectivityNode->numberOfChildren( ) )
        //                  << std::endl;

        float prevDist = interpolator.totalDistance( );

        //TODO Check if leaf section and add cut end
        auto infoSection = _infoSections.find( currentSection->section( ));
        if ( infoSection != _infoSections.end( ) &&
             std::get< tsi_leafSection >( infoSection->second ))
        {
          auto fixedSection = std::get< tsi_fixedSection >(
            infoSection->second );

          interpolator.insert( fixedSection );

//          std::cout << "Fixed ";
//          for( auto node : fixedSection )
//            std::cout << node << std::endl;
        }
        else
        {
          for ( auto node: currentSection->section( )->nodes( ))
          {
            auto point = transformPoint( node->point( ) , transform );

            interpolator.insert( point );
          }

          if ( last && last->numberOfChildren( ) > 1 )
          {
            //          auto children = last->children( );
            //          for( auto child = children.begin( ) + 1; child != children.end( ); ++child )
            {
              interpolator.addEventNode( prevDist , last->section( )->id( ));
            }
          }
        }

        if ( infoSection != _infoSections.end( ))
        {
          const auto& sectionSynapses = std::get< tsi_Synapses >(
            infoSection->second );
          for ( auto synapse: sectionSynapses )
          {
            auto it = _synapseFixInfo->find( synapse.first );
            if ( it != _synapseFixInfo->end( ))
            {
              auto presynInfo = std::get< TBSI_PRESYNAPTIC >( it->second );

              float synapseDist =
                std::get< TBS_SEGMENT_DISTANCE >( presynInfo );


              unsigned int segmentIndex = std::get< TBS_SEGMENT_INDEX >(
                presynInfo );
              const auto& sectionInterp =
                std::get< tsi_Interpolator >( infoSection->second );

              float sectionDist = sectionInterp.distance( segmentIndex );

              uintptr_t intPointer = reinterpret_cast< uintptr_t >( synapse.first );

              std::cout << "Adding synapse event " << synapse.first->gid( )
                        << " segment dist " << synapseDist
                        << " section dist " << sectionDist
                        << " current dist " << prevDist
                        << std::endl;

              synapseDist += prevDist;
              synapseDist += sectionDist;

              if ( _pathsPost.find( synapse.first ) == _pathsPost.end( ) &&
                   synapse.first->synapseType( ) !=
                   nsol::MorphologySynapse::AXOSOMATIC )
              {
                auto postSections = pathToSoma( synapse.first , POSTSYNAPTIC );
                // TODO cut leaf section (first)

                auto postTransform = getTransform(
                  synapse.first->postSynapticNeuron( ));

                tPosVec points;

                auto fixedSection = _infoSections.find( postSections.front( ));
                if ( fixedSection == _infoSections.end( ))
                {
                  std::cerr << "ERROR: Postsynaptic section "
                            << postSections.front( )->id( )
                            << " info not found." << std::endl;
                  continue;
                }

                auto sectionEndNodes =
                  std::get< tsi_fixedSection >( fixedSection->second );

                for ( auto nodeIt = sectionEndNodes.rbegin( );
                      nodeIt != sectionEndNodes.rend( ); ++nodeIt )
                  points.emplace_back( *nodeIt );

                for ( unsigned int i = 1; i < postSections.size( ); ++i )
//                for( auto sec : postSections )
                {
                  auto sec = postSections[ i ];

                  for ( auto nodeit = sec->nodes( ).rbegin( );
                        nodeit != sec->nodes( ).rend( ); ++nodeit )
//                    for( auto nodeit = sec->nodes( ).begin( ); nodeit != sec->nodes( ).end( ); ++nodeit )
                    points.emplace_back(
                      transformPoint(( *nodeit )->point( ) , postTransform ));
                }

                _pathsPost.insert( std::make_pair( synapse.first , points ));

                std::cout << "Added post path " << synapse.first
                          << " with " << points.size( ) << std::endl;

              }

              if ( synapseDist > interpolator.totalDistance( ))
              {
                std::cout << "--Fixing synapse dist " << synapseDist
                          << " to " << interpolator.totalDistance( )
                          << std::endl;

                synapseDist = interpolator.totalDistance( );
              }
              interpolator.addEventNode( synapseDist , intPointer ,
                                         utils::TEvent_synapse );
            }
          }
        }

        currentSection->sectionLength(
          interpolator.totalDistance( ) - prevDist );

        last = currentSection;
      }

      auto it = _computedPaths.insert(
        std::make_pair( origin->section( )->id( ) ,
                        interpolator ));

      return it.first->second;
    }
  }

  utils::EventPolylineInterpolation
  PathFinder::getPostsynapticPath( nsolMSynapse_ptr synapse ) const
  {
    auto it = _pathsPost.find( synapse );
    if ( it == _pathsPost.end( ))
    {
      return utils::EventPolylineInterpolation( );
    }

    return it->second;
  }

  std::vector< vec3 >
  PathFinder::cutLeafSection( unsigned int /*sectionID*/ ) const
  {
    tPosVec result;

    return result;
  }
}
