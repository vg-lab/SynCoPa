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
#include <unordered_set>

// types.h should be the first file included to ensure
// GLEW is included before any other GL file
//#ifndef syncopa_SKIP_GLEW_INCLUDE
//#include <GL/glew.h>
//#endif

#include <nsol/nsol.h>

#include <Eigen/Eigen>

// GLM includes
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef Eigen::Vector3f vec3;
typedef Eigen::Vector4f vec4;
typedef Eigen::Matrix4f mat4;

typedef std::vector< vec3 > tPosVec;

namespace syncopa
{

  typedef nsol::NeuronMorphologySectionPtr nsolMSection_ptr;
  typedef nsol::MorphologySynapsePtr nsolMSynapse_ptr;

  typedef std::vector< nsolMSection_ptr > tsectionVec;
  typedef std::vector< nsolMSynapse_ptr > tsynapseVec;

  typedef std::vector< unsigned int > gidVec;
  typedef std::set< unsigned int > gidSet;
  typedef std::unordered_set< unsigned int > gidUSet;

  typedef Eigen::Vector3f vec3;
  typedef Eigen::Vector4f vec4;
  typedef Eigen::Matrix4f mat4;

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


  inline Eigen::Vector3f glmToEigen( const glm::vec3& value )
  {
    return vec3( value.x, value.y, value.z );
  }

  inline Eigen::Vector4f glmToEigen( const glm::vec4& value )
  {
    return vec4( value.x, value.y, value.z, value.w );
  }

  inline vec3 transformPoint( const vec3& point, const mat4& matrix )
  {
    vec4 transPoint( point.x( ), point.y( ), point.z( ), 1 );
    transPoint = matrix * transPoint;

    return transPoint.block< 3, 1 >( 0, 0 );
  }

  inline std::ostream& operator<<( std::ostream& stream, const vec3& vec )
  {
    return stream << "("<< vec.x( ) << ", " << vec.y( ) << ", " << vec.z( ) << ")";
  }

  inline std::ostream& operator<<( std::ostream& stream, const glm::vec3& vec )
  {
    return stream << "("<< vec.x << ", " << vec.y << ", " << vec.z << ")";
  }
}



#endif /* SRC_TYPES_H_ */
