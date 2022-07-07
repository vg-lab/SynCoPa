//
// Created by gaeqs on 14/06/22.
//

#include "DynamicPathGenerator.h"

#include <QDebug>

namespace syncopa
{

  void DynamicPathGenerator::walkSection(
    PathGeneratorGeneralData& general , PathGeneratorData& data )
  {
    float distance = 0;
    float displacement = general.step * general.velocity;

    // Iterate positions and events.
    while ( distance < data.section.totalDistance( ))
    {
      auto position = data.section.pointAtDistance( distance );
      general.maxTime = std::max( general.maxTime , data.time );

      general.particles.push_back(
        particle( position , data.postsynaptic , data.time ));

      manageEvents( general , data , distance );

      data.time += general.step;
      distance += displacement;
    }
  }

  void DynamicPathGenerator::manageEvents(
    PathGeneratorGeneralData& general ,
    const PathGeneratorData& data , float distance )
  {
    const auto events = data.section.eventsAt(
      distance , general.step * general.velocity );
    for ( const auto& event: events )
    {
      const auto id = std::get< 1 >( event );

      // New section reached
      if ( std::get< 2 >( event ) == utils::TEvent_section )
      {
        manageNewSection( general , data , id );
      }
        // Synapse reached!
      else
      {
        manageSynapse( general , data , id );
      }

    }
  }

  void DynamicPathGenerator::manageNewSection(
    PathGeneratorGeneralData& general ,
    const PathGeneratorData& data , uint64_t id )
  {
    const auto node = general.pathFinder.node( id ).second;
    if ( node == nullptr )
    {
      std::cerr << "Couldn't find section node " << id << "." << std::endl;
      return;
    }

    for ( const auto child: node->children( ))
    {
      const auto childId = child->section( )->id( );

      auto& set = data.visitedSections;
      if ( set.find( childId ) != set.end( )) continue;

      PathGeneratorData newData = data;
      newData.visitedSections.insert( childId );
      newData.section = general.pathFinder.computeDeepestPathFrom( childId );
      walkSection( general , newData );
    }
  }

  void DynamicPathGenerator::manageSynapse(
    PathGeneratorGeneralData& general ,
    const PathGeneratorData& data , uint64_t id )
  {
    const auto synapse = reinterpret_cast< nsolMSynapse_ptr >( id );

    if ( synapse->synapseType( ) == nsol::MorphologySynapse::AXOSOMATIC )
      return;

    const auto path = general.pathFinder.getPostsynapticPath( synapse );
    if ( path.empty( ))
    {
      std::cerr << "Empty post path on synapse " << synapse << "." << std::endl;
      return;
    }

    PathGeneratorData newData = data;
    newData.section = path;
    newData.postsynaptic = true;
    // Walk postsynaptic instantly
    walkSection( general , newData );
  }

  DynamicPathParticle
  DynamicPathGenerator::particle(
    const vec3& position , bool postsynaptic , float time )
  {
    DynamicPathParticle particle = DynamicPathParticle( );
    particle.position = eigenToGLM( position );
    particle.isPostsynaptic = postsynaptic ? 1.0f : 0.0f;
    particle.timestamp = time;
    return particle;
  }

  std::pair< std::vector< DynamicPathParticle > , float >
  DynamicPathGenerator::generateParticles(
    PathFinder& pathFinder , float step , float velocity )
  {
    PathGeneratorGeneralData general( pathFinder , step , velocity );

    for ( const auto& item: pathFinder.presynapticTrees( ))
    {
      const auto& tree = item.second;
      for ( const auto& rootNode: tree.rootNodes( ))
      {
        auto path = pathFinder.computeDeepestPathFrom(
          rootNode->section( )->id( ));

        PathGeneratorData data( path , false , 0.0 );
        walkSection( general , data );
      }
    }

    return std::make_pair( general.particles , general.maxTime );
  }
}
