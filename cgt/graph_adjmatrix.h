/*
 * CGTL - A graph template library for C++
 * ---------------------------------------
 * Copyright (C) 2009 Leandro Costa
 *
 * This file is part of CGTL.
 *
 * CGTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CGTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with CGTL. If not, see <http://www.gnu.org/licenses/>.
 */

/*!
 * \file cgt/graph_adjmatrix.h
 * \brief Contains the definition of an adjacency matrix represented graph
 * \author Leandro Costa
 * \date 2009
 */

#ifndef __CGTL__CGT_GRAPH_ADJMATRIX_H_
#define __CGTL__CGT_GRAPH_ADJMATRIX_H_

#include "graph_type.h"
#include "graph_node.h"
#include "graph_vertex.h"
#include "base/list.h"
#include "misc/debug.h"


namespace cgt
{
  /*!
   * \class _GraphAdjMatrix
   * \brief An adjacency matrix represented by a list of nodes. Each node has a list of adjacencies.
   * \author Leandro Costa
   * \date 2009
   *
   * An adjacency matrix is a list of nodes. Each node has a vertex
   * and a list of adjacencies. Each adjacency is represented by
   * a pair: an edge and a reference to the other node.
   *
   * Each _GraphAdjMatrix is a list of nodes and contains a list of edges.
   * We need 4 bytes to represent the graph's type too.
   *
   * An empty graph has only an emtpy node's list (12 bytes) and an empty edge's
   * list (12 bytes). So, considering the 4 bytes to represent tye type, we need
   * <b>28 bytes</b> to represent an empty graph.
   *
   * A node has a vertex and two adjacency lists. Each node needs 8 more bytes
   * since it belongs to a doubly-linked list. As the overhead of a _GraphVertex
   * is 0 and the size of an empty adjacency's list is 12, we need
   * <b>32 + sizeof (_TpVertex) bytes</b> to represent each node.
   *
   * Each edge is an item in the edge's list and generates adjacencies for its nodes.
   * The overhead of an edge is 8 bytes (references to its vertices), and it needs 8 more
   * bytes since it belongs to a doubly-linked list. If the graph is directed, each edge
   * generates 2 adjacencies. If the graph is undirected, each edge generates 4 adjacencies.
   * Each adjacency needs 16 bytes (8 for the references plus 8 for the list pointers), so,
   * for a directed graph, we need <b>48 + sizeof (_TpEdg) bytes</b> to represent each
   * edge. For an undirected graph we need <b>64 + sizeof (_TpEdg) bytes</b>.
   *
   * Let's say \b v is the number of vertices and \b e is the number of edges in
   * the graph. For each vertex we have a node, and for each edge we have an item
   * in the edge's list and 2 (if it's a directed graph) or 4 (if it's an undirected
   * graph) adjacencies.
   *
   * So, the total size of a directed graph is:
   * <b>24 + v * (32 + sizeof (_TpVertex)) + e * (48 + sizeof (_TpEdge))</b>.
   *
   * And the total size of an undirected graph is:
   * <b>24 + v * (32 + sizeof (_TpVertex)) + e * (64 + sizeof (_TpEdge))</b>.
   */

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    class _GraphAdjMatrix : protected cgt::base::list<_GraphNode<_TpVertex, _TpEdge> >
  {
    private:
      typedef _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType> _Self;

    private:
      typedef _GraphNode<_TpVertex, _TpEdge>  _Node;
      typedef _GraphEdge<_TpVertex, _TpEdge>  _Edge;
      typedef _GraphVertex<_TpVertex>         _Vertex;
      typedef cgt::base::list<_Node>          _Base;
      typedef cgt::base::list<_Edge>          _EdgeList;

    protected:
      /**
       * A method to get a node pointer by its vertex.
       * @param _vertex a reference to the vertex.
       * @return A pointer to the node (NULL if the vertex is not found in graph).
       */
      _Node* _get_node (const _TpVertex &_vertex);

      typename _Base::iterator _find (const _TpVertex &_vertex);

    protected:
      void _insert_node (const _TpVertex &_vertex);
      void _insert_vertex (const _TpVertex &_vertex);
      void _insert_edge (const _TpEdge &_e, const _TpVertex &_v1, const _TpVertex &_v2);
      void _insert_edge (_Node& _n1, _Node& _n2, _Vertex& _v1, _Vertex& _v2, _Edge& _e);

    protected:
      void _invert ();

    protected:
      const bool _is_directed () const;
      const bool _is_undirected () const;

    private:
      _TpGraphType  _type; /** < the graph type: directed or undirected */

    protected:
      /*
       * We need a list of edges for two reasons:
       *  - first, that's an easy way to create the edge_iterator,
       *    since we only need to export a list iterator to users;
       *  - second, we use a reference to these edges in the 
       *    adjacency matrix. This way we can create only one
       *    edge and use it twice in undirected graphs.
       */

      _EdgeList _edgeList;
  };


  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    _GraphNode<_TpVertex, _TpEdge>* _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_get_node (const _TpVertex &_vertex)
    {
      _Node *_ptr_node = NULL;

      typename _Base::iterator it;
      typename _Base::iterator itEnd = _Base::end ();

      for (it = _Base::begin (); it != itEnd; ++it)
      {
        if (it->vertex () == _vertex)
        {
          _ptr_node = &(*it);
          break;
        }
      }

      return _ptr_node;
    }

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    typename cgt::base::list<_GraphNode<_TpVertex, _TpEdge> >::iterator _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_find (const _TpVertex &_vertex)
    {
      typename _Base::iterator it    = _Base::begin ();
      typename _Base::iterator itEnd = _Base::end ();

      while (it != itEnd && it->vertex () != _vertex)
        it++;
     
      return it;
    }

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    void _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_insert_node (const _TpVertex &_vertex)
    {
      insert (_GraphNode<_TpVertex, _TpEdge> (_vertex));
    }

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    void _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_insert_vertex (const _TpVertex &_vertex)
    {
      _Node *_ptr = _get_node (_vertex);

      if (! _ptr)
        _insert_node (_vertex);
      else
        _BRK();
    }

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    void _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_insert_edge (const _TpEdge &_e, const _TpVertex &_v1, const _TpVertex &_v2)
    {
      _Node *_ptr_n1 = _get_node (_v1);

      if (_ptr_n1)
      {
        _Node *_ptr_n2 = _get_node (_v2);

        if (_ptr_n2)
        {
          _Vertex& _vertex2 = _ptr_n2->vertex ();

          if (! _ptr_n1->_get_edge (_vertex2))
          {
            _Vertex& _vertex1 = _ptr_n1->vertex ();
            _Edge &_edge = _edgeList.insert (_Edge (_e, _vertex1, _vertex2));
            _insert_edge (*_ptr_n1, *_ptr_n2, _vertex1, _vertex2, _edge);
          }
        }
      }
    }

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    void _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_insert_edge (_Node& _n1, _Node& _n2, _Vertex& _v1, _Vertex& _v2, _Edge& _e)
    {
      _n1._insert (_e, _n2);

      if (! _type._directed && ! _n2._get_edge (_v1))
        _n2._insert (_e, _n1);
    }

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    void _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_invert ()
    {
      /*
       * for each node, call _invert_edges ()
       */

      typename _Base::iterator it;
      typename _Base::iterator itEnd = _Base::end ();

      for (it = _Base::begin (); it != itEnd; ++it)
        it->_invert_edges ();
    }

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    const bool _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_is_directed () const
    {
      return _type._directed;
    }

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType>
    const bool _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>::_is_undirected () const
    {
      return (! _is_directed ());
    }
}

#endif
