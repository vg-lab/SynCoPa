/*
 * @file  ConnectivityTree.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_CONNECTIVITYTREE_H_
#define SRC_CONNECTIVITYTREE_H_

#include "types.h"

#include <nsol/nsol.h>

#include "PolylineInterpolation.hpp"

namespace syncopa
{

  class ConnectivityNode;

  typedef ConnectivityNode* cnode_ptr;
  typedef std::vector< cnode_ptr > tCNodeVec;

  class ConnectivityNode
  {
  public:

    ConnectivityNode( nsolMSection_ptr section );

    void parent( cnode_ptr parent_ );
    cnode_ptr parent( void ) const;

    void sectionLength( float length_ );
    float sectionLength( void ) const;

    void addChild( cnode_ptr );
    cnode_ptr child( nsol::NeuronMorphologySectionPtr section_ );

    cnode_ptr firstChild( void ) const;
    cnode_ptr lastChild( void ) const;

    unsigned int numberOfChildren( void ) const;
    tCNodeVec children( void ) const;
    tCNodeVec allChildren( void ) const;
    tCNodeVec allFirstChildren( void ) const;

    tCNodeVec deepestPath( bool fromRoot = true ) const;
    cnode_ptr deepestChild( void ) const;
    cnode_ptr deepestBranchNode( void ) const;

    void section( nsol::NeuronMorphologySectionPtr section_ );
    nsol::NeuronMorphologySectionPtr section( void ) const;

    void initializeInterpolator( const mat4& transform );

    unsigned int childrenMaxDepth( void ) const;

  protected:

    ConnectivityNode( void );

    void _calculateDeepestPath( void );

    void _propagateDepth( const cnode_ptr sourceChild );

    nsol::NeuronMorphologySectionPtr _section;

    float _sectionLength;

    cnode_ptr _parent;

    std::unordered_map< nsol::NeuronMorphologySectionPtr, cnode_ptr > _children;

    unsigned int _maxDepth;
    cnode_ptr _deepestChildrenNode;
    cnode_ptr _deepestChild;

    utils::PolylineInterpolation _interpolator;
  };


  class ConnectivityTree
  {

  public:

    ConnectivityTree( );

    void clear( );

    unsigned int addBranch( const std::vector< nsolMSection_ptr >& sections );

    tCNodeVec rootNodes( void ) const;
    tCNodeVec leafNodes( void ) const;

    size_t size( void ) const;
    unsigned int maxDepth( void ) const;

    cnode_ptr node( nsol::NeuronMorphologySectionPtr section_) const;
    cnode_ptr node( unsigned int sectionID ) const;

    bool hasNode( nsol::NeuronMorphologySectionPtr section_ ) const;

    void print( void ) const;

  protected:

    unsigned int _size;
    unsigned int _maxDepth;

    tCNodeVec _rootNodes;
    tCNodeVec _leafNodes;

    std::unordered_map< unsigned int, cnode_ptr > _sectionIDToNodes;
    std::unordered_map< nsol::NeuronMorphologySectionPtr, cnode_ptr > _sectionToNodes;


  };


}



#endif /* SRC_CONNECTIVITYTREE_H_ */
