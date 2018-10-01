/*
 * @file  ConnectivityTree.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#include "ConnectivityTree.h"

namespace syncopa
{

  ConnectivityNode::ConnectivityNode( nsolMSection_ptr section_ )
  : _section( section_ )
  , _sectionLength( 0.0f )
  , _parent( nullptr )
  , _maxDepth( 0 )
  , _deepestChildrenNode( nullptr )
  , _deepestChild( nullptr )
  { }

  void ConnectivityNode::parent( cnode_ptr parent_ )
  {
    _parent = parent_;
  }

  cnode_ptr ConnectivityNode::parent( void ) const
  {
    return _parent;
  }

  void ConnectivityNode::sectionLength( float length_ )
  {
    _sectionLength = length_;
  }

  float ConnectivityNode::sectionLength( void ) const
  {
    return _sectionLength;
  }



//  void ConnectivityNode::checkDeepestBranch( unsigned int depth, cnode_ptr section )
//  {
//    if( depth > _maxDepth )
//    {
//      _maxDepth = depth;
//      _deepestChildrenNode = section;
//    }
//  }

  cnode_ptr ConnectivityNode::deepestBranchNode( void ) const
  {
    return _deepestChildrenNode;
  }

  cnode_ptr ConnectivityNode::deepestChild( void ) const
  {
    return _deepestChild;
  }

  unsigned int ConnectivityNode::childrenMaxDepth( void ) const
  {
    return _maxDepth;
  }

  void ConnectivityNode::initializeInterpolator( const mat4& transform )
  {
    if( _section )
    {
      for( auto node : _section->nodes( ))
        _interpolator.insert( transformPoint( node->point( ), transform ));
    }
  }

  void ConnectivityNode::section( nsol::NeuronMorphologySectionPtr section_ )
  {
    _section = section_;
  }

  nsol::NeuronMorphologySectionPtr ConnectivityNode::section( void ) const
  {
    return _section;
  }

  void ConnectivityNode::addChild( cnode_ptr child )
  {
    if( _children.empty( ))
    {
      _deepestChildrenNode = child;
      _deepestChild = child;
      _maxDepth = 1;

      if( _parent )
      {
        _propagateDepth( this );
      }
    }

    _children.insert( std::make_pair( child->section( ), child ));
  }

  tCNodeVec ConnectivityNode::children( void ) const
  {
    tCNodeVec result;
    result.reserve( _children.size( ));

    for( auto child : _children )
      result.push_back( child.second );

    return result;
  }

  cnode_ptr ConnectivityNode::firstChild( void ) const
  {
    if( _children.empty( ))
      return nullptr;
    else
      return _children.begin( )->second;
  }

  cnode_ptr ConnectivityNode::lastChild( void ) const
  {
    if( _children.empty( ))
      return nullptr;
    else
      // Assuming only two children
      return ( ++_children.begin( ))->second;
  }

  unsigned int ConnectivityNode::numberOfChildren( void ) const
  {
    return _children.size( );
  }

  void ConnectivityNode::_propagateDepth( const cnode_ptr sourceChild )
  {
    unsigned int totalChildDepth = sourceChild->childrenMaxDepth( ) + 1;

//    std::cout << "-propagating depth from " << sourceChild->section( )->id( )
//              << " old " << _maxDepth
//              << " new " << totalChildDepth
//              << std::endl;

    if( totalChildDepth >= _maxDepth  )
    {
      _maxDepth = totalChildDepth;
      _deepestChildrenNode = sourceChild->deepestBranchNode( );
      _deepestChild = sourceChild;

      if( _parent )
        _parent->_propagateDepth( this );
    }
  }

  void ConnectivityNode::_calculateDeepestPath( void )
  {
//    cnode_ptr deepestChild = nullptr;
//    auto nodeChildren = children( );
//    while( !nodeChildren.empty( ) )
//    {
//
//      deepestChild = nodeChildren.front( );
//
//      if( nodeChildren.size( ) > 1 && deepestChild->childrenMaxDepth( ) >
//                                      nodeChildren.back( )->childrenMaxDepth( ))
//      {
//        deepestChild = nodeChildren.back( );
//      }
//
//      result.push_back( deepestChild );
//
//      nodeChildren = deepestChild->children( );
//    }



  }

  tCNodeVec ConnectivityNode::deepestPath( bool fromRoot ) const
  {
    tCNodeVec result;
//TODO
    auto currentNode = _deepestChildrenNode;

    while( currentNode && currentNode->section( )->id( ) != _section->id( ))
    {
      result.push_back( currentNode );

      currentNode = currentNode->parent( );
    }

    if( fromRoot )
      std::reverse( result.begin( ), result.end( ));

//    std::cout  << std::endl << "Calculated deepest path with " << result.size( ) << " sections";

    return result;
  }

  tCNodeVec ConnectivityNode::allFirstChildren( void ) const
  {
    tCNodeVec result;

    auto nodeChildren = children( );

    std::cout << _section->id( ) << "Nodes ";

    while( !nodeChildren.empty( ))
    {
      auto node = nodeChildren.front( );

      result.push_back( node );

      std::cout << " " << node->section( )->id( );

      nodeChildren = node->children( );
    }

    std::cout << std::endl;

    return result;
  }

  tCNodeVec ConnectivityNode::allChildren( void ) const
  {
    std::queue< cnode_ptr > pendingNodes;

    tCNodeVec result;

    for( auto child : _children )
      pendingNodes.push( child.second );

    while( !pendingNodes.empty( ))
    {
      auto current = pendingNodes.front( );
      auto children = current->children( );
      for( auto child : children )
        pendingNodes.push( child );

      result.push_back( current );

      pendingNodes.pop( );
    }

    return result;
  }



  ConnectivityTree::ConnectivityTree(  )
  : _size( 0 )
  , _maxDepth( 0 )
  { }

  unsigned int ConnectivityTree::addBranch( const std::vector< nsolMSection_ptr >& sections )
  {

    std::cout << this << " Adding branch of " << sections.size( ) << " sections" << std::endl;

    if( sections.empty( ))
      return 0;

    auto rootSection = sections.back( );

    cnode_ptr rootNode;
    auto rootSecIt = _sectionToNodes.find( rootSection );
    if( rootSecIt == _sectionToNodes.end( ))
    {
      rootNode = new ConnectivityNode( rootSection );
      rootNode->parent( nullptr );

      _rootNodes.push_back( rootNode );
      _sectionToNodes.insert( std::make_pair( rootSection, rootNode ));
      _sectionIDToNodes.insert( std::make_pair( rootSection->id( ), rootNode ));

      _size += 1;

    }
    else
      rootNode = rootSecIt->second;


    cnode_ptr last = rootNode;
    cnode_ptr node = nullptr;
//    unsigned int i = 0;
//    for( int i = sections.size( ) - 2; i >= 0; --i )
    for( auto sec = sections.rbegin( ) + 1; sec != sections.rend( ); ++sec )
    {
//      auto sec = sections[ i ];
//
//      if( i == 0 )
//        continue;
      auto it = _sectionToNodes.find( *sec );
      if( it == _sectionToNodes.end( ))
      {
        node = new ConnectivityNode( *sec );
        node->parent( last );

        it = _sectionToNodes.insert( std::make_pair( *sec, node )).first;
        _sectionIDToNodes.insert( std::make_pair( ( *sec )->id( ), node ));

        _size += 1;
      }
      else
        node = it->second;
//      ++i;
      last->addChild( node );

      last = node;

    }

    if( rootNode->childrenMaxDepth( ) + 1 > _maxDepth )
      _maxDepth = rootNode->childrenMaxDepth( ) + 1;

    return sections.size( );

  }

  size_t ConnectivityTree::size( void ) const
  {
    return _size;
  }

  unsigned int ConnectivityTree::maxDepth( void ) const
  {
    return _maxDepth;
  }


  tCNodeVec ConnectivityTree::rootNodes( void ) const
  {
    tCNodeVec result;
    result.reserve( _rootNodes.size( ));

    for( auto node : _rootNodes )
    {
      result.push_back( node );
    }

    return result;
  }

  tCNodeVec ConnectivityTree::leafNodes( void ) const
  {
    tCNodeVec result;

    std::queue< cnode_ptr> pending;

    for( auto node : _rootNodes )
    {
      pending.push( node );
    }

    cnode_ptr currentNode;
    while( !pending.empty( ))
    {
      currentNode = pending.front( );

      if( currentNode->numberOfChildren( ) > 0 )
      {
        for( auto child : currentNode->children( ))
        {
          pending.push( child );
        }
      }
      else
      {
        result.push_back( currentNode );
      }

      pending.pop( );
    }

    return result;
  }

  cnode_ptr ConnectivityTree::node( nsol::NeuronMorphologySectionPtr section_) const
  {
    auto it = _sectionToNodes.find( section_ );
    if( it == _sectionToNodes.end( ))
      return nullptr;
    else
      return it->second;
  }

  cnode_ptr ConnectivityTree::node( unsigned int sectionID ) const
  {
    auto it = _sectionIDToNodes.find( sectionID );
    if( it == _sectionIDToNodes.end( ))
      return nullptr;
    else
      return it->second;
  }


  bool ConnectivityTree::hasNode( nsol::NeuronMorphologySectionPtr section_ ) const
  {
    return _sectionToNodes.find( section_ ) != _sectionToNodes.end( );
  }

  void ConnectivityTree::clear( )
  {
    _size = 0;
    _maxDepth = 0;
    _rootNodes.clear( );
    _sectionToNodes.clear( );
    _sectionIDToNodes.clear( );
  }

  void ConnectivityTree::print( void ) const
  {
    cnode_ptr currentNode = nullptr;

    std::queue< cnode_ptr > pending;

    for( auto rootNode : _rootNodes )
    {
      pending.push( rootNode );
    }

    while( !pending.empty( ))
    {
      currentNode = pending.front( );

      if( !currentNode->parent( ))
        std::cout << "Root ";
      else
        std::cout << "Node ";

      std::cout << currentNode->section( )->id( )
                << " children (" << currentNode->numberOfChildren( ) << ") ";

      for( auto child : currentNode->children( ))
      {
        std::cout << child->section( )->id( ) << " ";

        pending.push( child );
      }

//      if( currentNode->numberOfChildren( ) > 2 )
      {
//        std::cout << " -> (" << currentNode->section( )->forwardNeighbors( ).size( ) << ") ";

        for( auto child : currentNode->deepestPath( ))
//        for( auto child : currentNode->section( )->forwardNeighbors( ))
        {
          std::cout  << " " << child->section( )->id( );
        }
      }

      std::cout << std::endl;

      pending.pop( );
    }
  }


}
