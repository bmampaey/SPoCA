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
 * \file cgt/graph.h
 * \brief Contains graph definition. The only file that needs to be included in source code.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-11 02:53:57 +0100 (Fri, 11 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 104 $
 */

#ifndef __CGTL__CGT_GRAPH_H_
#define __CGTL__CGT_GRAPH_H_

#include "graph_type.h"
#include "graph_vertex.h"
#include "graph_edge.h"
#include "graph_adjacency.h"
#include "graph_node.h"
#include "graph_adjlist.h"
#include "graph_adjmatrix.h"
#include "vertex_iterator.h"

#include "search/depth/depth_iterator.h"
#include "search/breadth/breadth_iterator.h"


/*!
 * \namespace cgt
 * \brief The primary namespace where are defined graph and its main elements.
 * \author Leandro Costa
 * \date 2009
 */

namespace cgt
{
  /*!
   * \class graph
   * \brief The graph class template.
   * \author Leandro Costa
   * \date 2009
   *
   * A graph with vertices of type \b _TpVertex, edges of type \b _TpEdge.
   * The type \b _TpGraphType is used to represent directed (\b _Directed)
   * and indirected (\b _Undirected) graphs.
   *
   * \code
   *  _______________________________________________________________________________________________________________________________________________________________
   * | graph                                                                                                                                                         |
   * |_______________________________________________________________________________________________________________________________________________________________|
   * |  ___________________________________________________________________________________________________________________________________________________________  |
   * | | _GraphNodeList                                                                                                                                            | |
   * | |___________________________________________________________________________________________________________________________________________________________| |
   * | |               __________________________________________________________________      __________________________________________________________________  | |
   * | | _GraphNode*  | _GraphNode                                                       |    | _GraphNode                                                       | | |
   * | | _GraphNode*  |__________________________________________________________________| -> |__________________________________________________________________| | |
   * | | size_t       |  ______________                                                  |    |  ______________                                                  | | |
   * | |              | | _GraphVertex |                                                 |    | | _GraphVertex |                                                 | | |
   * | |              | |______________|                                                 |    | |______________|                                                 | | |
   * | |              |                                                                  |    |                                                                  | | |
   * | |              |  ______________________________________________________________  |    |  ______________________________________________________________  | | |
   * | |              | | _GraphAdjList                                                | |    | | _GraphAdjList                                                | | | |
   * | |              | |______________________________________________________________| |    | |______________________________________________________________| | | |
   * | |              | |                    _________________      _________________  | |    | |                    _________________      _________________  | | | |
   * | |              | | _GraphAdjacency*  | _GraphAdjacency |    | _GraphAdjacency | | |    | | _GraphAdjacency*  | _GraphAdjacency |    | _GraphAdjacency | | | | |
   * | |              | | _GraphAdjacency*  |_________________| -> |_________________| | |    | | _GraphAdjacency*  |_________________| -> |_________________| | | | |
   * | |              | | size_t            |                 |    |                 | | |    | | size_t            |                 |    |                 | | | | |
   * | |              | |                   |   _GraphEdge&   |    |   _GraphEdge&   | | |    | |                   |   _GraphEdge&   |    |   _GraphEdge&   | | | | |
   * | |              | |                   |   _GraphNode&   | <- |   _GraphNode&   | | |    | |                   |   _GraphNode&   | <- |   _GraphNode&   | | | | |
   * | |              | |                   |_________________|    |_________________| | |    | |                   |_________________|    |_________________| | | | |
   * | |              | |______________________________________________________________| |    | |______________________________________________________________| | | |
   * | |              |                                                                  |    |                                                                  | | |
   * | |              |  ______________________________________________________________  |    |  ______________________________________________________________  | | |
   * | |              | | _GraphAdjList                                                | |    | | _GraphAdjList                                                | | | |
   * | |              | |______________________________________________________________| |    | |______________________________________________________________| | | |
   * | |              | |                    _________________      _________________  | |    | |                    _________________      _________________  | | | |
   * | |              | | _GraphAdjacency*  | _GraphAdjacency |    | _GraphAdjacency | | |    | | _GraphAdjacency*  | _GraphAdjacency |    | _GraphAdjacency | | | | |
   * | |              | | _GraphAdjacency*  |_________________| -> |_________________| | |    | | _GraphAdjacency*  |_________________| -> |_________________| | | | |
   * | |              | | size_t            |                 |    |                 | | |    | | size_t            |                 |    |                 | | | | |
   * | |              | |                   |   _GraphEdge&   |    |   _GraphEdge&   | | |    | |                   |   _GraphEdge&   |    |   _GraphEdge&   | | | | |
   * | |              | |                   |   _GraphNode&   | <- |   _GraphNode&   | | |    | |                   |   _GraphNode&   | <- |   _GraphNode&   | | | | |
   * | |              | |                   |_________________|    |_________________| | |    | |                   |_________________|    |_________________| | | | |
   * | |              | |______________________________________________________________| | <- | |______________________________________________________________| | | |
   * | |              |__________________________________________________________________|    |__________________________________________________________________| | |
   * | |___________________________________________________________________________________________________________________________________________________________| |
   * |                                                                                                                                                               |
   * |  __________________________________________________________________________                                                                                   |
   * | | _GraphEdgeList                                                           |                                                                                  |
   * | |__________________________________________________________________________|                                                                                  |
   * | |               _______________      _______________      _______________  |                                                                                  |
   * | | _GraphEdge*  | _GraphEdge    |    | _GraphEdge    |    | _GraphEdge    | |                                                                                  |
   * | | _GraphEdge*  |_______________| -> |_______________| -> |_______________| |                                                                                  |
   * | | size_t       |               |    |               |    |               | |                                                                                  |
   * | |              | _TpEdge       |    | _TpEdge       |    | _TpEdge       | |                                                                                  |
   * | |              | _GraphVertex& |    | _GraphVertex& |    | _GraphVertex& | |                                                                                  |
   * | |              | _GraphVertex& | <- | _GraphVertex& | <- | _GraphVertex& | |                                                                                  |
   * | |              |_______________|    |_______________|    |_______________| |                                                                                  |
   * | |__________________________________________________________________________|                                                                                  |
   * |                                                                                                                                                               |
   * |                                                                                                                                                               |
   * |  _TpGraphType                                                                                                                                                 |
   * |_______________________________________________________________________________________________________________________________________________________________|
   *
   * \endcode
   */

  template<typename _TpVertex, typename _TpEdge, typename _TpGraphType = _Directed>
    class graph : private _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType>
  {
    private:
      typedef _GraphAdjMatrix<_TpVertex, _TpEdge, _TpGraphType> _Base;

    public:
      typedef _GraphVertex<_TpVertex>           vertex;
      typedef _GraphEdge<_TpVertex, _TpEdge>    edge;
      typedef _GraphNode<_TpVertex, _TpEdge>    node;
      typedef _GraphAdjList<_TpVertex, _TpEdge> adjlist;

      typedef typename _Base::iterator          iterator;
      typedef typename _Base::const_iterator    const_iterator;

    public:
      void insert_vertex (const _TpVertex &_vertex) { _insert_vertex (_vertex); }
      void insert_edge (const _TpEdge &_e, const _TpVertex &_v1, const _TpVertex &_v2) { _insert_edge (_e, _v1, _v2); }

    public:
      const node* get_node (const _TpVertex &_vertex) { return _Base::_get_node (_vertex); }
      iterator find (const _TpVertex &_vertex) { return _Base::_find (_vertex); }

    public:
      /**
       * A method to generate the transpose of graph.
       * Time complexity: O(V).
       */
      void invert () { _Base::_invert (); }

    public:
      const bool is_directed () const { return _Base::_is_directed (); }
      const bool is_undirected () const { return _Base::_is_undirected (); }

    public:
      /** node iterator */
      iterator begin () { return _Base::begin (); }
      iterator end () { return _Base::end (); }
      const_iterator begin () const { return _Base::begin (); }
      const_iterator end () const { return _Base::end (); }

      /** vertex iterator */
      typedef _VertexIterator<_TpVertex, _TpEdge>                                viterator;
      typedef _VertexIterator<_TpVertex, _TpEdge, cgt::base::iterator::_TpConst> const_viterator;

      viterator vbegin () { return viterator (_Base::begin ()); }
      viterator vend () { return viterator (_Base::end ()); }
      const_viterator vbegin () const { return const_viterator (_Base::begin ()); }
      const_viterator vend () const { return const_viterator (_Base::end ()); }

      /** edge iterator */
      typedef typename cgt::base::list<edge>::iterator       eiterator;
      typedef typename cgt::base::list<edge>::const_iterator const_eiterator;

      eiterator ebegin () { return eiterator (_Base::_edgeList.begin ()); }
      eiterator eend () { return eiterator (_Base::_edgeList.end ()); }
      const_eiterator ebegin () const { return const_eiterator (_Base::_edgeList.begin ()); }
      const_eiterator eend () const { return const_eiterator (_Base::_edgeList.end ()); }

      /** depth-first search iterator */
      typedef cgt::search::depth::_DepthIterator<_TpVertex, _TpEdge>                                 diterator;
      typedef cgt::search::depth::_DepthIterator<_TpVertex, _TpEdge, cgt::base::iterator::_TpConst>  const_diterator;
      typedef typename diterator::_DepthInfo                                                         depth_info;
      typedef typename cgt::base::list<depth_info>::iterator                                         diiterator;
      typedef typename cgt::base::list<depth_info>::const_iterator                                   const_diiterator;

      diterator dbegin () { return diterator (_Base::begin (), _Base::begin (), _Base::end ()); }
      diterator dbegin (const iterator& _it) { return diterator (_it, _Base::begin (), _Base::end ()); }
      diterator dend () { return diterator (NULL); }
      const_diterator dbegin () const { return const_diterator (_Base::begin (), _Base::begin (), _Base::end ()); }
      const_diterator dbegin (const iterator& _it) const { return const_diterator (_it, _Base::begin (), _Base::end ()); }
      const_diterator dend () const { return const_diterator (NULL); }

      diiterator dibegin (diterator &_it) { return diiterator (_it.info_begin ()); }
      diiterator diend (diterator &_it) { return diiterator (_it.info_end ()); }
      const_diiterator dibegin (diterator &_it) const { return const_diiterator (_it.info_begin ()); }
      const_diiterator diend (diterator &_it) const { return const_diiterator (_it.info_end ()); }

      /** breadth-first search iterator */
      typedef cgt::search::breadth::_BreadthIterator<_TpVertex, _TpEdge>                                 biterator;
      typedef cgt::search::breadth::_BreadthIterator<_TpVertex, _TpEdge, cgt::base::iterator::_TpConst>  const_biterator;
      typedef typename biterator::_BreadthInfo                                                           breadth_info;
      typedef typename cgt::base::list<breadth_info>::iterator                                           biiterator;
      typedef typename cgt::base::list<breadth_info>::const_iterator                                     const_biiterator;

      biterator bbegin () { return biterator (_Base::begin (), _Base::begin (), _Base::end ()); }
      biterator bbegin (const iterator& _it) { return biterator (_it, _Base::begin (), _Base::end ()); }
      biterator bend () { return biterator (NULL); }
      const_biterator bbegin () const { return const_biterator (_Base::begin (), _Base::begin (), _Base::end ()); }
      const_biterator bbegin (const iterator& _it) const { return const_biterator (_it, _Base::begin (), _Base::end ()); }
      const_biterator bend () const { return const_biterator (NULL); }

      biiterator bibegin (biterator &_it) { return biiterator (_it.info_begin ()); }
      biiterator biend (biterator &_it) { return biiterator (_it.info_end ()); }
      const_biiterator bibegin (biterator &_it) const { return const_biiterator (_it.info_begin ()); }
      const_biiterator bibegin (const_biterator &_it) const { return const_biiterator (_it.info_begin ()); }
      const_biiterator biend (biterator &_it) const { return const_biiterator (_it.info_end ()); }
      const_biiterator biend (const_biterator &_it) const { return const_biiterator (_it.info_end ()); }

     
  };
}

#endif // __CGTL__CGT_GRAPH_H_
