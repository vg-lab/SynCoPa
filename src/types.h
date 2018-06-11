/*
 * @file  types.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_TYPES_H_
#define SRC_TYPES_H_

#include <iostream>
#include <string>
#include <vector>
#include <set>

// types.h should be the first file included to ensure
// GLEW is included before any other GL file
//#ifndef SYNVIS_SKIP_GLEW_INCLUDE
//#include <GL/glew.h>
//#endif

#include <Eigen/Eigen>

// GLM includes
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace synvis
{
  typedef Eigen::Vector3f vec3;
  typedef Eigen::Vector4f vec4;
  typedef Eigen::Matrix4f mat4;

  typedef std::vector< unsigned int > gidVec;
  typedef std::set< unsigned int > gidSet;

  enum TNeuronConnection
  {
    PRESYNAPTIC = 0,
    POSTSYNAPTIC,
    ALL_CONNECTIONS
  };

  enum TApplicationMode
  {
    SYNAPSE = 0,
    PATH_FROM_TO,
    PATH_DEPTH
  };

  inline glm::vec3 eigenToGLM( const Eigen::Vector3f& value )
  {
    return glm::vec3( value.x( ), value.y( ), value.z( ));
  }

  inline glm::vec4 eigenToGLM( const Eigen::Vector4f& value )
  {
    return glm::vec4( value.x( ), value.y( ), value.z( ), value.w( ));
  }

  inline Eigen::Vector4f glmToEigen( const glm::vec4& value )
  {
    return vec4( value.x, value.y, value.z, value.w );
  }
}



#endif /* SRC_TYPES_H_ */
