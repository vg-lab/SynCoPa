//
// Created by gaeqs on 22/3/22.
//

#ifndef SYNCOPA_CAMERA_H
#define SYNCOPA_CAMERA_H

#include <prefr/prefr.h>
#include <reto/reto.h>

class Camera : public prefr::ICamera, public reto::OrbitalCameraController
{

  static glm::vec3 floatPtrToVec3( float* floatPos );

  static glm::mat4x4 floatPtrToMat4( float* floatPos );

public:



  glm::vec3 PReFrCameraPosition( ) override;

  glm::mat4x4 PReFrCameraViewMatrix( ) override;

  glm::mat4x4 PReFrCameraViewProjectionMatrix( ) override;

};


#endif //SYNCOPA_CAMERA_H
