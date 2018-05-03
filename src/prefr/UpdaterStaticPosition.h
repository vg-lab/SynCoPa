/*
 * @file  UpdaterStaticPosition.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_PREFR_UPDATERSTATICPOSITION_H_
#define SRC_PREFR_UPDATERSTATICPOSITION_H_

#include "../types.h"

#include <prefr/prefr.h>

namespace synvis
{
  class UpdaterStaticPosition : public prefr::Updater
  {
  public:

    UpdaterStaticPosition( );
    ~UpdaterStaticPosition( );

    void updateParticle( prefr::tparticle current, float deltaTime );


  };


}



#endif /* SRC_PREFR_UPDATERSTATICPOSITION_H_ */
