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
 * \file cgt/search/depth/depth_iterator.h
 * \brief Contains definition of an iterator that implements depth-first search algorithm.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-02 00:05:33 +0100 (Wed, 02 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 96 $
 */

#ifndef __CGTL__CGT_SEARCH_DEPTH_DEPTH_ITERATOR_H_
#define __CGTL__CGT_SEARCH_DEPTH_DEPTH_ITERATOR_H_

#include "../search_iterator.h"
#include "../../base/stack.h"


namespace cgt
{
  namespace search
  {
    /*!
     * \namespace cgt::search::depth
     * \brief Where are defined structures related to depth-first search algorithms.
     * \author Leandro Costa
     * \date 2009
     */

    namespace depth
    {
      /*!
       * \class _DepthIterator
       * \brief An iterator that implements depth-first search by nodes.
       * \author Leandro Costa
       * \date 2009
       *
       * depth-first search algorithm
       *  - _it_node: iterator pointing to the begining of the node list;
       *  - _it_node_end: iterator pointing to the end of the node list.
       *
       * initialize:
       *  - paint the first node with GRAY;
       *  - put it on the stack;
       *  - turn it the current node;
       *  - paint all the others with WHITE.
       *
       * operator++:
       *  - visit the adjacency list of the stack's top node:
       *    - if a WHITE node is found:
       *      - paint it with GRAY, put it on the stack and turn it the current node.
       *    - if there is no more WHITE nodes:
       *      - pop the stack's top node, paint it with BLACK;
       *      - continue visiting the adjacency list of the new stack's top node
       *  - if the stack is empty:
       *    - use _it_node to find the next WHITE node.
       *    - if a WHITE node is found:
       *      - paint it with GRAY, put it on the stack and turn it the current node.
       *    - if no more WIHTE nodes are found:
       *      - point the current node to NULL.
       */

      template<typename _TpVertex, typename _TpEdge, template<typename> class _TpIterator = cgt::base::iterator::_TpCommon>
        class _DepthIterator : public _SearchIterator<_TpVertex, _TpEdge, cgt::base::stack, _TpIterator>
      {
        public:
          typedef _SearchInfo<_TpVertex, _TpEdge>   _DepthInfo;
          typedef _SearchState<_TpVertex, _TpEdge>  _DepthState;

        private:
          typedef _DepthIterator<_TpVertex, _TpEdge, _TpIterator> _Self;
          typedef _DepthIterator<_TpVertex, _TpEdge, cgt::base::iterator::_TpCommon>   _SelfCommon;
          typedef _GraphNode<_TpVertex, _TpEdge>                  _Node;
          typedef typename cgt::base::list<_Node>::iterator      _NodeIterator;

        private:
          typedef _SearchIterator<_TpVertex, _TpEdge, cgt::base::stack, _TpIterator> _Base;

        private:
          using _Base::_ptr_node;
          using _Base::_it_node;
          using _Base::_it_node_end;
          using _Base::_infoList;
          using _Base::_stContainer;
          using _Base::_global_time;

        public:
          _DepthIterator () { }
          _DepthIterator (_Node* const _ptr_n) : _Base (_ptr_n) { }
          _DepthIterator (_Node* const _ptr_n, const _NodeIterator& _it_begin, const _NodeIterator& _it_end) : _Base (_ptr_n, _it_begin, _it_end) { }
          _DepthIterator (const _NodeIterator& _it, const _NodeIterator& _it_begin, const _NodeIterator& _it_end) : _Base (&(*_it), _it_begin, _it_end) { }
          _DepthIterator (const _SelfCommon& _it) { }

        public:
          _Self& operator++();

        public:
          const _DepthInfo* const info (const _Node* const _ptr_node) { return _get_depth_info_by_node (_ptr_node); }
          const _DepthInfo* const info (const _Node& _node) { return _get_depth_info_by_node (_node); }

          typename cgt::base::list<_DepthInfo>::iterator info_begin () { return _infoList.begin (); }
          typename cgt::base::list<_DepthInfo>::iterator info_end () { return _infoList.end (); }
      };

      template<typename _TpVertex, typename _TpEdge, template<typename> class _TpIterator>
        _DepthIterator<_TpVertex, _TpEdge, _TpIterator>& _DepthIterator<_TpVertex, _TpEdge, _TpIterator>::operator++()
        {
          /*
           * visit the adjacency list of the stack's top node:
           *  - if a WHITE node is found:
           *    - paint it with GRAY, put it on the stack and turn it the current node.
           *  - if there is no more WHITE nodes:
           *    - pop the stack's top node, paint it with BLACK;
           *    - continue visiting the adjacency list of the new stack's top node
           * if the stack is empty:
           *  - use _it_node to find the next WHITE node.
           *  - if a WHITE node is found:
           *    - paint it with GRAY, put it on the stack and turn it the current node.
           *  - if no more WIHTE nodes are found:
           *    - point the current node to NULL.
           */

          _ptr_node = NULL;

          while (! _stContainer.empty ())
          {
            _DepthState *_ptr_state  = _stContainer.top ();

            while (! _ptr_state->adj_finished ())
            {
              if (_has_color (_ptr_state->_adj_node (), _DepthInfo::WHITE))
              {
                _ptr_node = &(_ptr_state->_adj_node ());
                _ptr_state->adj_incr ();
                _stContainer.push (_DepthState (*_ptr_node));
                _discover_node (*_ptr_node, &(_ptr_state->node ()), ++_global_time);
                break;
              }
              else
                _ptr_state->adj_incr ();
            }

            if (! _ptr_node)
            {
              _DepthState *_ptr = _stContainer.pop ();
              _finish_node (_ptr->node (), ++_global_time);
              delete _ptr;
            }
            else
              break;
          }

          if (! _ptr_node)
          {
            while (_it_node != _it_node_end && ! _has_color (*_it_node, _DepthInfo::WHITE))
              ++_it_node;

            if (_it_node != _it_node_end)
            {
              _ptr_node = &(*_it_node);
              _stContainer.push (_DepthState (*_it_node));
              _discover_node (*_it_node, NULL, ++_global_time);
            }
          }

          return *this;
        }
    }
  }
}

#endif // __CGTL__CGT_SEARCH_DEPTH_DEPTH_ITERATOR_H_
