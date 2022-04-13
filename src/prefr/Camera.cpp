//
// Created by gaeqs on 22/3/22.
//

#include "Camera.h"

glm::mat4x4 Camera::PReFrCameraViewProjectionMatrix( )
{
  return floatPtrToMat4( camera( )->projectionViewMatrix( ));
}

glm::mat4x4 Camera::PReFrCameraViewMatrix( )
{
  return floatPtrToMat4( camera( )->viewMatrix( ));
}

glm::vec3 Camera::PReFrCameraPosition( )
{
  return floatPtrToVec3( position( ).data( ));
}

glm::vec3 Camera::floatPtrToVec3( float* floatPos )
{
  return { floatPos[ 0 ] ,
           floatPos[ 1 ] ,
           floatPos[ 2 ] };
}

glm::mat4x4 Camera::floatPtrToMat4( float* floatPos )
{
  return { floatPos[ 0 ] , floatPos[ 1 ] ,
           floatPos[ 2 ] , floatPos[ 3 ] ,
           floatPos[ 4 ] , floatPos[ 5 ] ,
           floatPos[ 6 ] , floatPos[ 7 ] ,
           floatPos[ 8 ] , floatPos[ 9 ] ,
           floatPos[ 10 ] , floatPos[ 11 ] ,
           floatPos[ 12 ] , floatPos[ 13 ] ,
           floatPos[ 14 ] , floatPos[ 15 ] };
}
