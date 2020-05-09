#pragma once
#include <functional> 
#include <iostream>
#define COPY_PROTECT(classname_)   classname_(const classname_&)=delete;classname_& operator=(const classname_&)=delete;

template <class T> class ScopeGuard
{
	COPY_PROTECT(ScopeGuard);
	std::function<void(T)> _cleanup;
	T _entity;
public:
	ScopeGuard(): _entity(nullptr){};
	ScopeGuard(const T entity_, std::function<void(T)>const& cleanup_) {
		_cleanup = cleanup_;
		_entity = entity_;
	}
	~ScopeGuard() {
		if (_entity) {
			_cleanup(_entity);
		}
	}
	T& operator*() {
		return *_entity;
	}
	T* guarded() {
		return &_entity;
	}
	operator T() {
		return _entity;
	}
};

