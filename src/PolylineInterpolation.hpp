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
    : _size( 0 )
    {
      _distances.reserve( nodes.size( ));
      _positions.reserve( nodes.size( ));
      _directions.reserve( nodes.size( ));

      insert( nodes );
    }

    virtual ~PolylineInterpolation( void )
    { }

    virtual void insert( const PolylineInterpolation& other )
    {
      unsigned int totalSize = _size + other.size( );

      float offsetDistance = totalDistance( );
      unsigned int offsetIndex = _size;

      insert( other.firstPosition( ));

      _positions.resize( totalSize );
      _distances.resize( totalSize );
      _directions.resize( totalSize );

      for( unsigned int i = 1; i < other.size( ); ++i )
      {
        _positions[ offsetIndex + i ] = other._positions[ i ];
        _directions[ offsetIndex + i ] = other._directions[ i ];
        _distances[ offsetIndex + i ] = other._distances[ i ] + offsetDistance;
      }
    }

    virtual void insert( const std::vector< vec3 >& nodes )
    {

      if( nodes.empty( ))
        return;

      float accDist = 0;

      vec3 currentPoint;
      vec3 nextPoint;

      insert( nodes.front( ));

      accDist = _distances.back( );

      for( unsigned int i = 0; i < nodes.size( ) - 1; ++i )
      {
        currentPoint = nodes[ i ];
        nextPoint = nodes[ i + 1 ];

        vec3 dir = nextPoint - currentPoint;
        float module = dir.norm( );
        accDist += module;
        dir = dir / module;

        insert( accDist, nextPoint, dir );
      }
    }

    virtual void insert( const vec3 node )
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

    virtual inline unsigned int insert( float distance, vec3 position, vec3 direction )
    {
      if( !_positions.empty( ) && position == _positions.back( ))
        return _size;

//      unsigned int i = 0;
//
//      // Iterate over time values till the last minus one
//      while( i < _size && _size > 0 )
//      {
//        // Overwrite position
//        if (distance == _distances[ i ])
//        {
//          _positions[ i ] = position;
//          return _size;
//        }
//        // New intermediate position
//        else if( distance < _distances[ i ])
//        {
//          _distances.emplace( _distances.begin( ) + i, distance );
//          _positions.emplace( _positions.begin( ) + i, position );
//          _directions.emplace( _directions.begin( ) + i, direction );
//
////          precision = GetPrecision(distance);
////          precisionValues.emplace(precisionValues.begin() + i,
////                                  precision);
//
//          _size = ( unsigned int ) _distances.size( );
////          UpdateQuickReference(precision);
//          return i;
//        }
//        i++;
//      }

      // New highest position
      _distances.push_back( distance );
      _positions.push_back( position );
      _directions.push_back( direction );

//      precision = GetPrecision(distance);
//      precisionValues.push_back(precision);

      _size = (unsigned int) _distances.size();

      return _size - 1;

//      UpdateQuickReference(precision);
    }

    inline void clear()
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

    bool empty( void ) const
    {
      return ( _size == 0 );
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

    vec3 direction( unsigned int i ) const
    {
      return _directions[ i ];
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

    float distance( unsigned int i ) const
    {
      assert( i < _size );
      return _distances[ i ];
    }

    float totalDistance( void ) const
    {
      if( _size == 0 )
        return 0.0f;

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
    inline vec3 pointAtDistance( float distance ) const
    {
      if( _size > 1 && distance > 0.0f )
      {
       unsigned int i = segmentFromDistance( distance );

       return pointAtDistance( distance, i );
      }
      else
      {
        return _positions[ 0 ];
      }
    }

    inline vec3 pointAtDistance( float distance, unsigned int segmentIdx ) const
    {

      if( segmentIdx >= _size )
        segmentIdx = _size -1;

      float accumulated = distance - _distances[ segmentIdx ];

      vec3 dir = _directions[ segmentIdx + 1 ];

      vec3 res = _positions[ segmentIdx ] + dir * accumulated;

      return res;
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

    void reverse( void )
    {
      std::vector< vec3 > reversePositions;
      reversePositions.reserve( _positions.size( ));

      for( auto it = _positions.rbegin( ); it != _positions.rend( ); ++it )
      {
        reversePositions.push_back( *it );
      }

//      std::vector< vec3 > reverseDirections;
//      reverseDirections.reserve( _directions.size( ));
//      reverseDirections.push_back( _directions.front( ));
//
//      for( unsigned int i = _directions.size( ) - 1; i > 1 ; --i )
//      {
//        reverseDirections.push_back( - _directions[ i ]);
//      }
//
//      for( unsigned int i = 0; i < )

      clear( );
      insert( reversePositions );
    }

    unsigned int segmentFromDistance( float distance ) const
    {
      if( _distances.empty( ) || distance > _distances.back( ))
        return _size;

      unsigned int i = 0;
      while( distance > _distances[ i + 1 ])
      {
        ++i;
      }

      return i;
    }

  protected:

    std::vector< float > _distances;
    std::vector< vec3 > _positions;
    std::vector< vec3 > _directions;

    unsigned int _size;

  };

  enum tEventType
  {
    TEvent_section = 0,
    TEvent_synapse
  };

  typedef std::tuple< float, unsigned long int, unsigned int > tEventSectionInfo;
  typedef std::vector< tEventSectionInfo > tSectionEvents;

  class EventPolylineInterpolation : public PolylineInterpolation
  {
  public:

    EventPolylineInterpolation( )
    : PolylineInterpolation( )
    { }

    EventPolylineInterpolation( const tPosVec& nodes )
    : PolylineInterpolation( nodes )
    { }

    EventPolylineInterpolation( const PolylineInterpolation& other )
    : PolylineInterpolation( other )
    { }

    virtual void insert( const std::vector< vec3 >& nodes )
    {
      PolylineInterpolation::insert( nodes );
    }

    virtual void insert( const vec3 node )
    {
      PolylineInterpolation::insert( node );
    }

    void addEventNode( float distance, unsigned long int eventID, tEventType type = TEvent_section )
    {
      _events.push_back( std::make_tuple( distance, eventID, ( unsigned int ) type ));
//      unsigned int pos = _segmentFromDistance( distance );
//
//      if( pos >= _size - 1 )
//        return;
//
//      assert( _eventSection.size( ) == _size );
//
//      _eventSection[ pos ].push_back( std::make_pair( distance, eventID ));

//      std::cout << "--Added event for " << eventID
//                << " at " << pos
//                << " with dist " << distance
//                << std::endl;
//
//      std::cout << "Events: " << _distances[ pos ] << "-" << _distances[ pos + 1 ];
//      for( auto event : _eventSection[ pos ] )
//      {
//        std::cout << " " << event.first << "->" << event.second;
//      }
//      std::cout << std::endl;

    }

    tSectionEvents eventsAt( float distance, float step ) const
    {
      tSectionEvents result;

      float prevDist = std::max( 0.0f, distance - step );

      unsigned int i = segmentFromDistance( prevDist );

      if( i >= _size )
        return result;

      for( auto event : _events )
//      for( auto event : _eventSection[ i ] )
      {
        float eventDistance = std::get< 0 >( event );
        if( eventDistance >= prevDist && eventDistance < distance )
        {
          result.push_back( event );
        }
      }

      return result;
    }

  protected:


    tSectionEvents _events;
//    std::vector< tSectionEvents> _eventSection;


  };


}

#endif /* POLYLINEINTERPOLATION_H_ */
