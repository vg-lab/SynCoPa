/*
 * @file  UpdaterStaticPosition.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_PREFR_UPDATERMAPPEDVALUE_H_
#define SRC_PREFR_UPDATERMAPPEDVALUE_H_

#include "../types.h"

#include <prefr/prefr.h>

namespace syncopa
{
  class UpdaterMappedValue : public prefr::Updater
  {
  public:

    UpdaterMappedValue( void );
    ~UpdaterMappedValue( void );

    void updateParticle( prefr::tparticle current, float deltaTime );


  };


}



#endif /* SRC_PREFR_UPDATERMAPPEDVALUE_H_ */
