/*
 * @file  PolylineInterpolation.hpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_POLYLINEINTERPOLATION_HPP_
#define SRC_POLYLINEINTERPOLATION_HPP_

#include "types.h"

#include <vector>
#include <string>

#include <algorithm>

#include <iostream>

using namespace synvis;

namespace utils
{
  class PolylineInterpolation
  {
  public:

    PolylineInterpolation( void )
    : _size( 0 )
    { }

    PolylineInterpolation( const std::vector< vec3 >& nodes )
    : _size( nodes.size( ) )
    {
      distances.reserve( _size );
      positions.reserve( _size );
      directions.reserve( _size );

      insert( nodes );
    }

    void insert( const std::vector< vec3 >& nodes )
    {

      float accDist = 0;

      vec3 currentPoint;
      vec3 nextPoint;

      insert( 0, nodes.back( ), vec3( 0, 0, 0 ));

      for( unsigned int i = nodes.size( ) - 1; i > 1; --i )
      {
        currentPoint = nodes[ i ];
        nextPoint = nodes[ i - 1 ];

        vec3 dir = nextPoint - currentPoint;
        float module = dir.norm( );
        accDist += module;
        dir = dir / module;

        insert( accDist, nextPoint, dir );
      }
    }

    inline void insert( float distance, vec3 position, vec3 direction )
    {

      unsigned int i = 0;

      // Iterate over time values till the last minus one
      while (i < _size && _size > 0)
      {
        // Overwrite position
        if (distance == distances[i])
        {
          positions[i] = position;
          return;
        }
        // New intermediate position
        else if (distance < distances[i])
        {
          distances.emplace( distances.begin() + i, distance);
          positions.emplace( positions.begin() + i, position);
          directions.emplace( directions.begin() + i, direction);

//          precision = GetPrecision(distance);
//          precisionValues.emplace(precisionValues.begin() + i,
//                                  precision);

          _size = (unsigned int) distances.size();
//          UpdateQuickReference(precision);
          return;
        }
        i++;
      }

      // New highest position
      distances.push_back(distance);
      positions.push_back(position);
      directions.push_back(direction);

//      precision = GetPrecision(distance);
//      precisionValues.push_back(precision);

      _size = (unsigned int) distances.size();

//      UpdateQuickReference(precision);
    }

    inline void Clear()
    {
      distances.clear();
      positions.clear();
      directions.clear();
      _size = 0;
    }

    inline const vec3& GetFirstValue()
    {
//      if (!values.empty())
        return positions[0];

//      return defaultValue;
    }

    // Faster implementation for CPU interpolation.
    inline vec3 pointAtDistance(float distance)
    {
      if (_size > 1 && distance > 0.0f)
      {
       unsigned int i = 0;
       float accumulated = distance;
       while (distance > distances[i+1])
       {
         i++;
       }

       accumulated -= distances[i];

       vec3 dir = directions[i+1];


       vec3 res = positions[i] + dir * accumulated;

       return res;

      }
      else
      {
        return positions[0];
      }
    }

    std::vector< vec3 > interpolate( float stepSize,
                                     std::function< vec3 ( vec3 ) > operation )
    {
      float currentDist = 0;
      std::vector< vec3 > result;

      while( currentDist < distances.back( ))
      {
        vec3 pos = pointAtDistance( currentDist );

        result.push_back( operation( pos ));

        currentDist += stepSize;
      }

      return result;
    }

  protected:

    std::vector< float > distances;
    std::vector< vec3 > positions;
    std::vector< vec3 > directions;

    unsigned int _size;

  };




}

#endif /* POLYLINEINTERPOLATION_H_ */
