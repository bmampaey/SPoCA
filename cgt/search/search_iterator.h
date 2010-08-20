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
 * \file cgt/search/search_iterator.h
 * \brief Contains the base of breadth-first and depth-first iterators.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-01 01:01:04 +0100 (Tue, 01 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 90 $
 */

#ifndef __CGTL__CGT_SEARCH_SEARCH_ITERATOR_H_
#define __CGTL__CGT_SEARCH_SEARCH_ITERATOR_H_

#include "../graph_node.h"
#include "search_state.h"
#include "search_info.h"


namespace cgt
{
  /*!
   * \namespace cgt::search
   * \brief Where are defined structures related to breadth-first and depth-first search algorithms.
   * \author Leandro Costa
   * \date 2009
   */

  namespace search
  {
    /*!
     * \class _SearchIterator
     * \brief The base template for breadth-first and depth-first search iterators.
     * \author Leandro Costa
     * \date 2009
     *
     * The _SearchIterator is the base for breath and depth iterators. It defines
     * operations that are shared by both, and accepts a template class as a
     * container template that is initialized as a queue or a stack according
     * to the method of search.
     */

    template<typename _TpVertex, typename _TpEdge, template<typename> class _TpStateContainer, template<typename> class _TpIterator = cgt::base::iterator::_TpCommon>
      class _SearchIterator
      {
        private:
//          friend class _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, typename _TpIterator<_GraphNode<_TpVertex, _TpEdge> >::other>;
          friend class _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, cgt::base::iterator::_TpConst>;

        private:
          typedef _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator>                     _Self;
          typedef _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, cgt::base::iterator::_TpCommon>  _SelfCommon;

        private:
          typedef _SearchInfo<_TpVertex, _TpEdge>     _Info;
          typedef cgt::base::list<_Info>              _InfoList;
          typedef typename _InfoList::iterator        _InfoIterator;
          typedef typename _InfoList::const_iterator  _InfoCIterator;

        protected:
          typedef _GraphNode<_TpVertex, _TpEdge>      _Node;
          typedef typename cgt::base::list<_Node>     _NodeList;
          typedef typename _NodeList::iterator        _NodeIterator;
          typedef typename _NodeList::const_iterator  _NodeCIterator;

          typedef _SearchState<_TpVertex, _TpEdge>    _State;

        private:
          typedef typename _TpIterator<_Node>::pointer    pointer;
          typedef typename _TpIterator<_Node>::reference  reference;

        protected:
          _SearchIterator () : _ptr_node (NULL), _it_node (NULL), _it_node_end (NULL), _global_time (0) { }
          _SearchIterator (_Node* const _ptr_n) : _ptr_node (_ptr_n), _global_time (0) { }
          _SearchIterator (_Node* const _ptr_n, const _NodeIterator& _it_begin, const _NodeIterator& _it_end)
            : _ptr_node (_ptr_n), _it_node (_it_begin), _it_node_end (_it_end), _global_time (0)
          {
            _BRK();
            if (_ptr_node)
              _init ();
            _BRK();
          }

        public:
          _SearchIterator (const _SelfCommon& _it) { *this = _it; }

        protected:
          virtual ~_SearchIterator () { }

        public:
          const _Self& operator=(const _SelfCommon& _it)
          {
            _ptr_node     = _it._ptr_node;
            _it_node      = _it._it_node;
            _it_node_end  = _it._it_node_end;
            _global_time  = _it._global_time;
            _infoList     = _it._infoList;
            _stContainer  = _it._stContainer;

            return *this;
          }

        private:
          void _init ();

        protected:
          _Info* _get_depth_info_by_node (const _Node& _node);

        protected:
          void _discover_node (const _Node& _node, const _Node* const _ptr_parent, const unsigned long& _d);
          void _finish_node (const _Node& _node, const unsigned long& _f);
          const bool _has_color (const _Node& _node, const typename _Info::_color_t& _color) const;

        public:
          reference operator*() const { return *_ptr_node; }
          pointer operator->() const { return _ptr_node; }
          const bool operator==(const _Self& _other) const { return _ptr_node == _other._ptr_node; }
          const bool operator!=(const _Self& _other) const { return !(*this == _other); }
          const _Self operator++(int);

        protected:
          _Node*                    _ptr_node;
          _NodeIterator             _it_node;
          _NodeIterator             _it_node_end;
          _InfoList                 _infoList;
          _TpStateContainer<_State> _stContainer;

          unsigned long             _global_time;
      };

    template<typename _TpVertex, typename _TpEdge, template<typename> class _TpStateContainer, template<typename> class _TpIterator>
      void _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator>::_init ()
      {
        /*
         * paint the first node (_ptr_node) with GRAY;
         * put it on the state container;
         * turn it the current node (actualy, it already is the current node);
         * paint all the others with WHITE.
         */

        _NodeIterator _it;

        for (_it = _it_node; _it != _it_node_end; ++_it)
        {
          if (&(*_it) == _ptr_node)
          {
            _infoList.insert (_Info (*_it, _Info::GRAY, ++_global_time));
            _stContainer.insert (_State (*_it));
          }
          else
            _infoList.insert (_Info (*_it));
        }
      }

    template<typename _TpVertex, typename _TpEdge, template<typename> class _TpStateContainer, template<typename> class _TpIterator>
      typename _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator>::_Info* _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator>::_get_depth_info_by_node (const _Node& _node)
      {
        _Info *_ptr = NULL;

        _InfoIterator _it;
        _InfoIterator _itEnd = _infoList.end ();

        for (_it = _infoList.begin (); _it != _itEnd; ++_it)
        {
          if (_it->node ().vertex () == _node.vertex ())
          {
            _ptr = &(*_it);
            break;
          }
        }

        return _ptr;
      }

    template<typename _TpVertex, typename _TpEdge, template<typename> class _TpStateContainer, template<typename> class _TpIterator>
      void _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator>::_discover_node (const _Node& _node, const _Node* const _ptr_parent, const unsigned long& _d)
      {
        _Info *_ptr = _get_depth_info_by_node (_node);

        if (_ptr)
        {
          _ptr->set_parent (_ptr_parent);
          _ptr->set_color (_Info::GRAY);
          _ptr->set_discovery (_d);
        }
      }

    template<typename _TpVertex, typename _TpEdge, template<typename> class _TpStateContainer, template<typename> class _TpIterator>
      void _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator>::_finish_node (const _Node& _node, const unsigned long& _f)
      {
        _Info *_ptr = _get_depth_info_by_node (_node);

        if (_ptr)
        {
          _ptr->set_color (_Info::BLACK);
          _ptr->set_finish (_f);
        }
      }

    template<typename _TpVertex, typename _TpEdge, template<typename> class _TpStateContainer, template<typename> class _TpIterator>
      const bool _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator>::_has_color (const _Node& _node, const typename _Info::_color_t& _color) const
      {
        bool bRet = false;

        _InfoCIterator it;
        _InfoCIterator itEnd = _infoList.end ();

        for (it = _infoList.begin (); it != itEnd; ++it)
        {
          if (it->node ().vertex () == _node.vertex ())
          {
            bRet = (it->color () == _color);
            break;
          }
        }

        return bRet;
      }

    template<typename _TpVertex, typename _TpEdge, template<typename> class _TpStateContainer, template<typename> class _TpIterator>
      const _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator> _SearchIterator<_TpVertex, _TpEdge, _TpStateContainer, _TpIterator>::operator++(int)
      {
        _Self _it = *this;
        operator++();
        return _it;
      }
  }
}

#endif // __CGTL__CGT_SEARCH_SEARCH_ITERATOR_H_
