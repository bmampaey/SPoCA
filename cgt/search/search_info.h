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
 * \file cgt/search/search_info.h
 * \brief Contains a structure used to put informations about breadth-first and depth-first search iterators
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-01 01:01:04 +0100 (Tue, 01 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 90 $
 */

#ifndef __CGTL__CGT_SEARCH_SEARCH_INFO_H_
#define __CGTL__CGT_SEARCH_SEARCH_INFO_H_

#include "../graph_node.h"


namespace cgt
{
  namespace search
  {
    /*!
     * \class _SearchInfo
     * \brief The _SearchInfo class template.
     * \author Leandro Costa
     * \date 2009
     *
     * The _SearchInfo keeps informations about depth-first search and breadth-first
     * search visited node like color, parent, discovery time, finish time, etc.
     */

    template<typename _TpVertex, typename _TpEdge>
      class _SearchInfo
      {
        private:
          typedef _SearchInfo<_TpVertex, _TpEdge> _Self;

        private:
          typedef _GraphNode<_TpVertex, _TpEdge> _Node;

        public:
          typedef enum { WHITE, GRAY, BLACK } _color_t;

        public:
          _SearchInfo (_Node& _n) : _node (_n), _color (WHITE), _ptr_parent (NULL), _discovery (0), _finish (0) { }
          _SearchInfo (_Node& _n, const _color_t& _c, const unsigned long& _d) : _node (_n), _color (_c), _ptr_parent (NULL), _discovery (_d), _finish (0) { }
          _SearchInfo (const _Self& _s) : _node (_s._node), _color (_s._color), _ptr_parent (_s._ptr_parent), _discovery (_s._discovery), _finish (_s._finish) { }

        public:
          void set_parent (const _Node* const _ptr) { _ptr_parent = _ptr; }
          void set_color (const _color_t& _c) { _color = _c; }
          void set_discovery (const unsigned long& _d) { _discovery = _d; }
          void set_finish (const unsigned long& _f) { _finish = _f; }

          _Node& node () const { return _node; }
          const _Node* const parent () const { return _ptr_parent; }
          const _color_t& color () const { return _color; }
          const unsigned long& discovery () const { return _discovery; }
          const unsigned long& finish () const { return _finish; }

        private:
          _Node&        _node;
          _color_t      _color;
          const _Node*  _ptr_parent;
          unsigned long _discovery;
          unsigned long _finish;
      };
  }
}

#endif // __CGTL__CGT_SEARCH_SEARCH_INFO_H_
