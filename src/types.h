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

#include <QColor>

typedef Eigen::Vector3f vec3;
typedef Eigen::Vector4f vec4;
typedef Eigen::Matrix4f mat4;

typedef std::vector< vec3 > tPosVec;

typedef std::vector< std::pair< float, QColor >> tQColorVec;
typedef std::vector< std::pair< float, glm::vec4 >> tColorVec;

namespace syncopa
{

  typedef nsol::NeuronMorphologySectionPtr nsolMSection_ptr;
  typedef nsol::MorphologySynapsePtr nsolMSynapse_ptr;

  typedef std::vector< nsolMSection_ptr > tsectionVec;
  typedef std::vector< nsolMSynapse_ptr > tsynapseVec;

  typedef std::vector< unsigned int > gidVec;
  typedef std::set< unsigned int > gidSet;
  typedef std::unordered_set< unsigned int > gidUSet;
  typedef std::vector< float > tFloatVec;

  typedef Eigen::Vector3f vec3;
  typedef Eigen::Vector4f vec4;
  typedef Eigen::Matrix4f mat4;

  typedef std::tuple< unsigned int, unsigned int, float > tBrainSynapse;

  typedef std::tuple< float, float, float, float, float, float, int > tBrainSynapseAttribs;

  typedef std::tuple< tBrainSynapseAttribs, tBrainSynapse, tBrainSynapse > tSynapseData;

  typedef std::unordered_map< nsolMSynapse_ptr, tSynapseData > TSynapseInfo;

  enum TBrainSynapse
  {
    TBS_SECTION_ID = 0,
    TBS_SEGMENT_INDEX,
    TBS_SEGMENT_DISTANCE
  };

  enum TBrainSynapseAttribs
  {
    TBSA_SYNAPSE_DELAY = 0,
    TBSA_SYNAPSE_CONDUCTANCE,
    TBSA_SYNAPSE_UTILIZATION,
    TBSA_SYNAPSE_DEPRESSION,
    TBSA_SYNAPSE_FACILITATION,
    TBSA_SYNAPSE_DECAY,
    TBSA_SYNAPSE_EFFICACY,
    TBSA_SYNAPSE_OTHER
  };

  enum TBrainSynapseInfo
  {
    TBSI_ATTRIBUTES = 0,
    TBSI_PRESYNAPTIC,
    TBSI_POSTSYNAPTIC
  };

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

  const double invRGBNormFactor = 1.0 / 255;

  inline glm::vec4 qtToGLM( const QColor& value )
  {
    glm::vec4 result( value.red( ) * invRGBNormFactor,
                      value.green( ) * invRGBNormFactor,
                      value.blue( ) * invRGBNormFactor,
                      value.alpha( ) * invRGBNormFactor );

    result.x = std::min( std::max( 0.0f, result.x ), 1.0f );
    result.y = std::min( std::max( 0.0f, result.y ), 1.0f );
    result.z = std::min( std::max( 0.0f, result.z ), 1.0f );
    result.w = std::min( std::max( 0.0f, result.w ), 1.0f );

    return result;
  }

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

  inline float getSynapseAttribValue( const nsolMSynapse_ptr synapse,
                                      const tBrainSynapseAttribs& data,
                                      TBrainSynapseAttribs attrib )
  {
    float value;

    switch( attrib )
    {
      case TBSA_SYNAPSE_DELAY:
        value = std::get< TBSA_SYNAPSE_DELAY >( data );
        break;
      case TBSA_SYNAPSE_CONDUCTANCE:
        value = std::get< TBSA_SYNAPSE_CONDUCTANCE >( data );
        break;
      case TBSA_SYNAPSE_UTILIZATION:
        value = std::get< TBSA_SYNAPSE_UTILIZATION >( data );
        break;
      case TBSA_SYNAPSE_DEPRESSION:
        value = std::get< TBSA_SYNAPSE_DEPRESSION >( data );
        break;
      case TBSA_SYNAPSE_FACILITATION:
        value = std::get< TBSA_SYNAPSE_FACILITATION >( data );
        break;
      case TBSA_SYNAPSE_DECAY:
        value = std::get< TBSA_SYNAPSE_DECAY >( data );
        break;
      case TBSA_SYNAPSE_EFFICACY:
        value = std::get< TBSA_SYNAPSE_EFFICACY >( data );
        break;
      case TBSA_SYNAPSE_OTHER:
        value = ( unsigned int ) synapse->synapseType( );
        break;
    }

    return value;
  }
}



#endif /* SRC_TYPES_H_ */
