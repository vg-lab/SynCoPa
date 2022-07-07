//
// Created by gaeqs on 14/06/22.
//

#ifndef SYNCOPA_DYNAMICPATHGENERATOR_H
#define SYNCOPA_DYNAMICPATHGENERATOR_H


#include <memory>
#include <utility>
#include "PathFinder.h"
#include "particlelab/DynamicPathParticle.h"

namespace syncopa
{

  struct PathGeneratorGeneralData
  {
    PathFinder& pathFinder;
    std::vector< DynamicPathParticle > particles;
    float step;
    float velocity;
    float maxTime;

    PathGeneratorGeneralData(
      PathFinder& pathFinder_ , float step_ , float velocity_ )
      : pathFinder( pathFinder_ )
      , particles( )
      , step( step_ )
      , velocity( velocity_ )
      , maxTime( 0.0f )
    { };
  };

  struct PathGeneratorData
  {
    std::unordered_set< uint32_t > visitedSections;
    utils::EventPolylineInterpolation section;
    bool postsynaptic;
    float time;

    PathGeneratorData( utils::EventPolylineInterpolation section_ ,
                       bool postsynaptic_ , float time_ )
      : section( std::move( section_ ))
      , postsynaptic( postsynaptic_ )
      , time( time_ )
    { }
  };

  class DynamicPathGenerator
  {
    static void walkSection(
      PathGeneratorGeneralData& general , PathGeneratorData& data );

    static void manageEvents(
      PathGeneratorGeneralData& general ,
      const PathGeneratorData& data , float distance );

    static void manageNewSection(
      PathGeneratorGeneralData& general ,
      const PathGeneratorData& data , uint64_t id );

    static void manageSynapse(
      PathGeneratorGeneralData& general ,
      const PathGeneratorData& data , uint64_t id );

    static DynamicPathParticle particle(
      const vec3& position , bool postsynaptic , float time );

  public:

    static std::pair< std::vector< DynamicPathParticle > , float >
    generateParticles( PathFinder& pathFinder , float step , float velocity );

  };
}


#endif //SYNCOPA_DYNAMICPATHGENERATOR_H
