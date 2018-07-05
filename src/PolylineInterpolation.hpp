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

using namespace syncopa;

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
      _distances.reserve( _size );
      _positions.reserve( _size );
      _directions.reserve( _size );

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

    void insert( const vec3 node )
    {
      if( _size == 0 )
      {
        insert( 0, node, vec3( 0, 0, 0 ));
      }
      else
      {
        vec3 prevPoint = _positions.back( );
        float accDist = _distances.back( );

        vec3 dir = node - prevPoint;
        float module = dir.norm( );
        accDist += module;
        dir = dir / module;

        insert( accDist, node, dir );
      }
    }

    inline void insert( float distance, vec3 position, vec3 direction )
    {

      unsigned int i = 0;

      // Iterate over time values till the last minus one
      while (i < _size && _size > 0)
      {
        // Overwrite position
        if (distance == _distances[i])
        {
          _positions[i] = position;
          return;
        }
        // New intermediate position
        else if (distance < _distances[i])
        {
          _distances.emplace( _distances.begin() + i, distance);
          _positions.emplace( _positions.begin() + i, position);
          _directions.emplace( _directions.begin() + i, direction);

//          precision = GetPrecision(distance);
//          precisionValues.emplace(precisionValues.begin() + i,
//                                  precision);

          _size = (unsigned int) _distances.size();
//          UpdateQuickReference(precision);
          return;
        }
        i++;
      }

      // New highest position
      _distances.push_back(distance);
      _positions.push_back(position);
      _directions.push_back(direction);

//      precision = GetPrecision(distance);
//      precisionValues.push_back(precision);

      _size = (unsigned int) _distances.size();

//      UpdateQuickReference(precision);
    }

    inline void Clear()
    {
      _distances.clear();
      _positions.clear();
      _directions.clear();
      _size = 0;
    }

    inline const vec3& firstPosition() const
    {
//      if (!values.empty())
        return _positions[0];

//      return defaultValue;
    }

    const vec3& lastPosition( ) const
    {
      return _positions.back( );
    }

    const std::vector< vec3 >& positions( void ) const
    {
      return _positions;
    }


    size_t size( void ) const
    {
      return _size;
    }

    vec3 operator[]( unsigned int i ) const
    {
      assert( i < _size );
      return _positions[ i ];
    }

    vec3 segmentDirection( unsigned int i ) const
    {
      assert( i < _size - 1);
      return _directions[ i + 1 ];
    }

    vec3 lastSegmentDirection( void ) const
    {
      return _directions.back( );
    }

    float totalDistance( void ) const
    {
      return _distances.back( );
    }

    float segmentDistance( unsigned int i ) const
    {
      if( i < _size - 1)
        return _distances[ i + 1 ] - _distances[ i ];
      else
        return 0;
    }

    float firstSegmentDistance( void ) const
    {
      assert( _distances.size( ) > 1 );
      return _distances[ 1 ];
    }

    float lastSegmentDistance( void ) const
    {
      return _distances[ _size - 1 ] - _distances[ _size - 2 ];
    }

    // Faster implementation for CPU interpolation.
    inline vec3 pointAtDistance(float distance)
    {
      if (_size > 1 && distance > 0.0f)
      {
       unsigned int i = 0;
       float accumulated = distance;
       while (distance > _distances[i+1])
       {
         i++;
       }

       accumulated -= _distances[i];

       vec3 dir = _directions[i+1];


       vec3 res = _positions[i] + dir * accumulated;

       return res;

      }
      else
      {
        return _positions[0];
      }
    }

    std::vector< vec3 > interpolate( float stepSize,
                                     std::function< vec3 ( vec3 ) > operation )
    {
      float currentDist = 0;
      std::vector< vec3 > result;

      while( currentDist < _distances.back( ))
      {
        vec3 pos = pointAtDistance( currentDist );

        result.push_back( operation( pos ));

        currentDist += stepSize;
      }

      return result;
    }

  protected:

    std::vector< float > _distances;
    std::vector< vec3 > _positions;
    std::vector< vec3 > _directions;

    unsigned int _size;

  };




}

#endif /* POLYLINEINTERPOLATION_H_ */
