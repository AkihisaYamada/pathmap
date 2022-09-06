#ifndef PATHMAP_HPP_
#define PATHMAP_HPP_

#include<set>
#include<map>
#include<vector>
#include<iterator>


template< bool flag, class T1, class T2 >
struct cond_sig;

template< class T1, class T2 >
struct cond_sig<true,T1,T2> {
	typedef T1 type;
};
template< class T1, class T2 >
struct cond_sig<false,T1,T2> {
	typedef T2 type;
};
template< class L, class T >
class setmap;

// labelled tree
template< class L, class T, class P = std::vector<L> >
class pathmap {
protected:
public:
	typedef std::map< L, pathmap<L,T,P> > branch_type;
	typedef P path_type;
	typedef typename path_type::const_iterator path_iterator;
	template< bool flag >
	class iterator_base {
	private:
		typedef typename cond_sig< flag, pathmap<L,T,P> const, pathmap<L,T,P> >::type _M;
		std::vector<_M*> _parents;
		typedef typename cond_sig< flag, typename branch_type::const_iterator, typename branch_type::iterator >::type _I;
		std::vector<_I> _it_stack;
		typedef typename cond_sig< flag, T const, T >::type _Tc;
		void _seek();
		// for begin
		iterator_base( _M& map );

	public:
		iterator_base() {}
		iterator_base( iterator_base const& it ) :
			_parents(it._parents), _it_stack(it._it_stack) {}
		_Tc& operator*() const {
			return *_it_stack.back()->second._ptr;
		}
		_Tc* operator->() const {
			return _it_stack.back()->second._ptr;
		}
		bool operator== ( iterator_base const& it ) const {
			return _it_stack == it._it_stack;
		}
		bool operator!= ( iterator_base const& it ) const {
			return _it_stack != it._it_stack;
		}
		std::vector<L> path() const {
			std::vector<L> ret;
			for( auto& it_it : _it_stack ) {
				ret.push_back(it_it->first);
			}
			return ret;
		}
		iterator_base& operator++(int);
		operator bool() const {
			return !_it_stack.empty();
		}
		friend class pathmap<L,T,P>;
	};
	typedef iterator_base<false> iterator;
	typedef iterator_base<true> const_iterator;
private:
	T* _ptr;

	T& _get( path_iterator it, path_iterator end );
	pathmap* _find( path_iterator it, path_iterator end );
	pathmap const* _find( path_iterator it, path_iterator end ) const;
	bool _erase( path_iterator it, path_iterator end );
public:
	branch_type children;
	pathmap() : _ptr(NULL) {}
	pathmap( T const& v ) : _ptr( new T(v) ) {}
	~pathmap() {
		if( _ptr != NULL ) {
			delete _ptr;
		}
	}
	iterator begin() {
		return iterator(*this);
	}
	iterator end() {
		return iterator();
	}
	const_iterator begin() const {
		return const_iterator(*this);
	}
	const_iterator end() const {
		return const_iterator();
	}
	T& operator* () {
		return *_ptr;
	}
	T const& operator* () const {
		return *_ptr;
	}
	T* operator-> () {
		return _ptr;
	}
	T const* operator-> () const {
		return _ptr;
	}
	T& operator[] ( path_type const& path ) {
		return _get( path.begin(), path.end() );
	}
	T const& operator[] ( path_type const& path ) const {
		return **_find( path.begin(), path.end() );
	}
	T* find( path_type const& path ) {
		auto const p = _find( path.begin(), path.end() );
		return p ? p->_ptr : NULL;
	}
	T const* find( path_type const& path ) const {
		auto const p = _find( path.begin(), path.end() );
		return p ? p->_ptr : NULL;
	}
	bool contains( path_type const& path ) const {
		return find(path) != NULL;
	}
	bool allocated() const {
		return _ptr != NULL;
	}
	T& allocate( T const& val ) {
		if( _ptr == NULL ) {
			_ptr = new T(val);
		}
		return *this;
	}
	bool empty() const {
		return _ptr == NULL && children.empty();
	}
	bool erase() {
		if( _ptr != NULL ) {
			delete(_ptr);
			_ptr = NULL;
			return true;
		}
		return false;
	}
	bool erase( path_type const& path ) {
		return _erase( path.begin(), path.end() );
	}
	iterator erase( iterator& it );
	size_t size() const;
	std::vector<P> subsets( P const& p ) const {
		std::vector<P> ret;
		P path;
		subpaths_sub( *this, p.begin(), p.end(), path, ret );
		return ret;
	}
	std::vector<P> supersets( P const& p ) const {
		std::vector<P> ret;
		P path;
		superpaths_sub( *this, p.begin(), p.end(), path, ret );
		return ret;
	}
	template< bool flag >
	friend class iterator_base;
	friend class setmap<L,T>;
};

template< class L, class T >
class setmap : public pathmap< L, T, std::set<L> > {
public:
	typedef std::set<L> P, path_type;
	typedef pathmap<L,T,P> super;
	typedef typename path_type::const_iterator path_iterator;
protected:
	void erase_supersets( path_iterator it, path_iterator end );
	void erase_subsets( path_iterator it, path_iterator end );
public:
	std::vector<P> paths() const {
		std::vector<P> ret;
		P tmp;
		paths_sub(*this,ret,tmp);
		return ret;
	}
	// erase all sets that contain a subset
	void erase_supersets( path_type const& subset ) {
		erase_supersets( subset.begin(), subset.end() );
	}
	void erase_subsets( path_type const& superset ) {
		erase_subsets( superset.begin(), superset.end() );
	}
};


template< class L, class T, class P >
template< bool flag >
typename pathmap<L,T,P>::template iterator_base<flag>&
pathmap<L,T,P>::iterator_base<flag>::operator++ (int) {
	// look at my children
	_parents.push_back( &_it_stack.back()->second );
	_it_stack.push_back( _parents.back()->children.begin() );
	_seek();// find an allocated path
	return *this;
}
template< class L, class T, class P >
template< bool flag >
void pathmap<L,T,P>::iterator_base<flag>::_seek() {
	while( _it_stack.back() == _parents.back()->children.end() ) {
		// I have no more sibling. Look up my parent.
		_parents.pop_back();
		_it_stack.pop_back();
		if( _it_stack.empty() ) {
			return;// no parent anymore. Map traversed.
		}
		_it_stack.back()++;// This is an uncle.
	}
	while( !_it_stack.back()->second.allocated() ) {
		// I'm not allocated. Look at my children
		_parents.push_back( &_it_stack.back()->second );
		_it_stack.push_back( _parents.back()->children.begin() );
	}
}

template< class L, class T, class P >
template< bool flag >
pathmap<L,T,P>::iterator_base<flag>::iterator_base( _M& map ) {
	if( !map.children.empty() ) {
		// point to the first child
		_parents.push_back(&map);
		_it_stack.push_back( map.children.begin() );
		if( !_it_stack.back()->second.allocated() ) {
			// the child is not allocated, so look at the next one
			(*this)++;
		}
	}
}

template< class L, class T, class P >
T& pathmap<L,T,P>::_get(
	path_iterator it,
	path_iterator end
) {
	if( it == end ) {
		if( _ptr == NULL ) {
			_ptr = new T;
		}
		return *_ptr;
	}
	L const& l = *it;
	it++;
	return children[l]._get( it, end );
}
template< class L, class T, class P >
pathmap<L,T,P>* pathmap<L,T,P>::_find(
	path_iterator it,
	path_iterator end
) {
	pathmap<L,T,P>* cur = this;
	for( ;; it++ ) {
		if( it == end ) {
			return cur;
		}
		auto const& next_it = cur->children.find(*it);
		if( next_it == cur->children.end() ) {
			return NULL;
		}
		cur = &next_it->second;
	}
}
template< class L, class T, class P >
pathmap<L,T,P> const* pathmap<L,T,P>::_find(
	path_iterator it,
	path_iterator end
) const {
	pathmap<L,T,P> const* cur = this;
	for( ;; it++ ) {
		if( it == end ) {
			return cur;
		}
		auto const& next_it = cur->children.find(*it);
		if( next_it == cur->children.end() ) {
			return NULL;
		}
		cur = &next_it->second;
	}
}

template< class L, class T, class P >
bool pathmap<L,T,P>::_erase(
	path_iterator it,
	path_iterator end
) {
	if( it == end ) {
		return _erase();
	} else {
		auto& next = children.find(*it);
		if( next == children.end() ) {
			return false;
		} else {
			it++;
			bool ret = next->second.erase( it, end );
			if( next->second.empty() ) {// the child is needless anymore
				children.erase(next);
			}
			return ret;
		}
	}
}
template< class L, class T, class P >
typename pathmap<L,T,P>::iterator pathmap<L,T,P>::erase( iterator& it ) {
	it._it_stack.back()->second.erase();
	while( it._it_stack.back()->second.empty() ) {
		auto& branch_it = it._it_stack.back();
		it._it_stack.back()++;// look at the next sibling
		it._parents.back()->children.erase(branch_it);// erase me
		if( it._it_stack.back() == it._parents.back()->children.end() ) {
			// no more sibling. look up my parent.
			it._it_stack.pop_back();
			it._parents.pop_back();
			if( it._it_stack.empty() ) {
				return it;// no more element
			}
		}
	}
	it._seek();// look at the next element.
	return it;
}


template< class L, class T, class P >
size_t pathmap<L,T,P>::size() const {
	size_t ret = allocated() ? 1 : 0;
	for( auto& ch_it : children ) {
		ret += ch_it->second.size();
	}
	return ret;
}


template< class L, class T >
void setmap<L,T>::erase_subsets(
	path_iterator it,
	path_iterator end
) {
	if( it == end ) {
		super::_erase();
	} else {
		path_iterator next = it;
		next++;
		for( auto& ch_it : super::children ) {
			if( ch_it->first > *it ) {
				break;
			}
			if( ch_it->first < *it ) {
				ch_it->second.erase_subsets( it, end );
				continue;
			}
			ch_it->second.erase_subsets( next, end );
			if( ch_it->second.empty() ) {
				super::children.erase(ch_it);
			}
		}
	}
}

template< class L, class T >
void paths_sub(
	pathmap< L, T, std::set<L> > const& space,
	std::vector< std::set<L> >& paths,
	std::set<L>& prefix
) {
	if( space.allocated() ) {
		paths.push_back(prefix);
	}
	for( auto& ch_it : space.children ) {
		auto& l_it = prefix.insert( ch_it->first ).first;
		paths_sub( ch_it->second, paths, prefix );
		prefix.erase( l_it );
	}
}


template< class L, class T, class P >
void subpaths_sub(
	pathmap< L, T, P > const& space,
	typename P::const_iterator it,
	typename P::const_iterator end,
	P& prefix,
	std::vector<P>& paths
) {
	if( space.allocated() ) {
		paths.push_back(prefix);
	}
	if( it == end ) {
		return;
	}
	auto next = it;
	next++;
	for( auto& ch_it : space.children ) {
		while( ch_it->first > *it ) {
			it = next;
			next++;
			if( it == end ) {
				return;
			}
		}
		if( ch_it->first == *it ) {
			prefix.push_back( ch_it->first );
			subpaths_sub( ch_it->second, next, end, prefix, paths );
			prefix.pop_back();
		}
	}
}template< class L, class T >
void subpaths_sub(
	pathmap< L, T, std::set<L> > const& space,
	typename std::set<L>::iterator it,
	typename std::set<L>::iterator end,
	typename std::set<L>& prefix,
	std::vector< typename std::set<L> >& paths
) {
	if( space.allocated() ) {
		paths.push_back(prefix);
	}
	if( it == end ) {
		return;
	}
	auto next = it;
	next++;
	for( auto& ch_it : space.children ) {
		while( ch_it->first > *it ) {
			it = next;
			next++;
			if( it == end ) {
				return;
			}
		}
		if( ch_it->first == *it ) {
			auto& l_it = prefix.insert( ch_it->first ).first;
			subpaths_sub( ch_it->second, next, end, prefix, paths );
			prefix.erase( l_it );
		}
	}
}
template< class L, class T >
void superpaths_sub(
	pathmap< L, T, std::set<L> > const& space,
	typename std::set<L>::iterator it,
	typename std::set<L>::iterator end,
	typename std::set<L>& prefix,
	std::vector< typename std::set<L> >& paths
) {
	if( it == end ) {
		paths_sub( space, paths, prefix );
	} else {
		for( auto& ch_it : space.children ) {
			if( ch_it->first > *it ) {
				break;
			}
			auto& label_it = prefix.insert(ch_it->first).first;
			if( ch_it->first == *it ) {
				it++;
			}
			superpaths_sub( ch_it->second, it, end, prefix, paths );
			prefix.erase(label_it);
		}
	}
}

#endif
