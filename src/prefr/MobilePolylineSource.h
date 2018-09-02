/*
 * @file  MobileSource.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_PREFR_MOBILEPOLYLINESOURCE_H_
#define SRC_PREFR_MOBILEPOLYLINESOURCE_H_

#include "../types.h"

#include <boost/signals2.hpp>

#include <nsol/nsol.h>
#include <prefr/prefr.h>

#include "../ConnectivityTree.h"
#include "../PolylineInterpolation.hpp"

namespace syncopa
{

  class MobilePolylineSource : public prefr::Source
  {
  public:

    typedef boost::signals2::signal< void ( unsigned int )> tSigSourceID;
    typedef tSigSourceID::slot_type tSigSourceIDSlot;


    typedef boost::signals2::signal< void ( unsigned int )> tSigSection;

    typedef tSigSection::slot_type tSigSectionSlot;

    typedef boost::signals2::signal< void ( unsigned long int )> tSigSynapse;
    typedef tSigSynapse::slot_type tSigSynapseSlot;


    MobilePolylineSource( float emissionRate,
                          const vec3& position_ );

    void path( const tPosVec& nodes );
    void path( const utils::PolylineInterpolation& interpolator );
    void path( const utils::EventPolylineInterpolation& interpolator );

    void delay( float delay_ );

    void velocityModule( float module );

    float currentTime( void ) const;

    unsigned int gid( void ) const;

    void addEventNode( float distance, cnode_ptr section );

    void initialNode( cnode_ptr section_ );
    cnode_ptr initialNode( void ) const;

    virtual void _checkEmissionEnd( );
    virtual void _checkFinished( );

    virtual void restart( );

    virtual void prepareFrame( const float& deltaTime );
//    virtual void closeFrame( );

    tSigSourceID finishedPath;

    tSigSection finishedSection;

    tSigSynapse reachedSynapse;

  protected:

    static unsigned int _counter;

    unsigned int _gid;

//    cnode_ptr _initialNode;

    virtual void updatePosition( float deltaTime );

    float _currentDistance;

    float _velocityModule;

    float _delay;
    float _currentTime;

    utils::EventPolylineInterpolation _interpolator;

  };

}



#endif /* SRC_PREFR_MOBILEPOLYLINESOURCE_H_ */
