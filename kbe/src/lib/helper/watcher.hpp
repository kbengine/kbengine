/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __KBE_HELPER_WATCHER_HPP__
#define __KBE_HELPER_WATCHER_HPP__

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/memorystream.hpp"

namespace KBEngine
{

namespace Mercury{
}

typedef uint16 WATCHER_ID;
typedef uint8 WATCHERTYPE;

#define WATCHER_TYPE_UNKNOWN			0
#define WATCHER_TYPE_UINT8				1
#define WATCHER_TYPE_UINT16				2
#define WATCHER_TYPE_UINT32				3
#define WATCHER_TYPE_UINT64				4
#define WATCHER_TYPE_INT8				5
#define WATCHER_TYPE_INT16				6
#define WATCHER_TYPE_INT32				7
#define WATCHER_TYPE_INT64				8
#define WATCHER_TYPE_FLOAT				9
#define WATCHER_TYPE_DOUBLE				10
#define WATCHER_TYPE_CHAR				11
#define WATCHER_TYPE_STRING				12
#define WATCHER_TYPE_BOOL				13
#define WATCHER_TYPE_COMPONENT_TYPE		14

/*
	watcher基础对象
*/
class WatcherObject
{
public:
	WatcherObject(std::string name):
	  name_(name),
	  s_()
	{
	}
	
	virtual ~WatcherObject(){
	}
	
	virtual void addToInitStream(MemoryStream* s) = 0;

	virtual void addToStream(MemoryStream* s) = 0;

	virtual void updateStream(MemoryStream* s) = 0;

	WATCHER_ID id(){ return id_; }
	void id(WATCHER_ID i){ id_ = i; }

	const char* name(){ return name_.c_str(); }

	template <class T>
	WATCHERTYPE type()const{ return WATCHER_TYPE_UNKNOWN; }
protected:
	std::string name_;
	WATCHER_ID id_;
	MemoryStream s_;
};

template <>
inline WATCHERTYPE WatcherObject::type<uint8>()const
{
	return WATCHER_TYPE_UINT8;
}

template <>
inline WATCHERTYPE WatcherObject::type<uint16>()const
{
	return WATCHER_TYPE_UINT16;
}

template <>
inline WATCHERTYPE WatcherObject::type<uint32>()const
{
	return WATCHER_TYPE_UINT32;
}

template <>
inline WATCHERTYPE WatcherObject::type<uint64>()const
{
	return WATCHER_TYPE_UINT64;
}

template <>
inline WATCHERTYPE WatcherObject::type<int8>()const
{
	return WATCHER_TYPE_INT8;
}

template <>
inline WATCHERTYPE WatcherObject::type<int16>()const
{
	return WATCHER_TYPE_INT16;
}

template <>
inline WATCHERTYPE WatcherObject::type<int32>()const
{
	return WATCHER_TYPE_INT32;
}

template <>
inline WATCHERTYPE WatcherObject::type<int64>()const
{
	return WATCHER_TYPE_INT64;
}

template <>
inline WATCHERTYPE WatcherObject::type<float>()const
{
	return WATCHER_TYPE_FLOAT;
}

template <>
inline WATCHERTYPE WatcherObject::type<double>()const
{
	return WATCHER_TYPE_DOUBLE;
}

template <>
inline WATCHERTYPE WatcherObject::type<bool>()const
{
	return WATCHER_TYPE_BOOL;
}

template <>
inline WATCHERTYPE WatcherObject::type<char*>()const
{
	return WATCHER_TYPE_STRING;
}

template <>
inline WATCHERTYPE WatcherObject::type<std::string>()const
{
	return WATCHER_TYPE_STRING;
}

template <>
inline WATCHERTYPE WatcherObject::type<COMPONENT_TYPE>()const
{
	return WATCHER_TYPE_COMPONENT_TYPE;
}

/*
	watcher: 直接监视一个值
*/
template <class T>
class WatcherValue : public WatcherObject
{
public:
	WatcherValue(std::string name, const T& pVal):
	WatcherObject(name),
	watchVal_(pVal)
	{
	}
	
	virtual ~WatcherValue(){
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << id_ << type<T>() << watchVal_;
	};

	void addToStream(MemoryStream* s){
		(*s) << id_ << watchVal_;
	};

	virtual void updateStream(MemoryStream* s){
		s_.clear(false);
		s_.append(s->data() + s->rpos(), sizeof(T));
		s->read_skip<T>();
	}

protected:
	const T& watchVal_;
	T val_;
};

/*
	watcher: 监视一个方法返回的值
*/
template <class RETURN_TYPE>
class WatcherFunction : public WatcherObject
{
public:
	typedef RETURN_TYPE(*FUNC)();

	WatcherFunction(std::string name, RETURN_TYPE (*func)()):
	WatcherObject(name),
	func_(func)
	{
	}
	
	virtual ~WatcherFunction(){
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << id_ << type<RETURN_TYPE>() << (*func_)();
	};

	void addToStream(MemoryStream* s)
	{
		(*s) << id_ << (*func_)();
	};

	virtual void updateStream(MemoryStream* s){
		s_.clear(false);
		s_.append(s->data() + s->rpos(), sizeof(RETURN_TYPE));
		s->read_skip<RETURN_TYPE>();
	}

protected:
	FUNC func_;
};

/*
	watcher: 监视一个成员函数返回的值
*/
template <class RETURN_TYPE, class OBJ_TYPE>
class WatcherMethod : public WatcherObject
{
public:
	typedef RETURN_TYPE(OBJ_TYPE::*FUNC)();

	WatcherMethod(std::string name, OBJ_TYPE* This, RETURN_TYPE (OBJ_TYPE::*func)()):
	WatcherObject(name),
	func_(func),
	This_(This)
	{
	}
	
	virtual ~WatcherMethod(){
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << id_ << type<RETURN_TYPE>() << (This_->*func_)();
	};

	void addToStream(MemoryStream* s)
	{
		(*s) << id_ << (This_->*func_)();
	};

	virtual void updateStream(MemoryStream* s){
		s_.clear(false);
		s_.append(s->data() + s->rpos(), sizeof(RETURN_TYPE));
		s->read_skip<RETURN_TYPE>();
	}

protected:
	FUNC func_;
	OBJ_TYPE* This_;
};

template <class RETURN_TYPE, class OBJ_TYPE>
class WatcherMethodConst : public WatcherObject
{
public:
	typedef RETURN_TYPE(OBJ_TYPE::*FUNC)()const;

	WatcherMethodConst(std::string name, OBJ_TYPE* This, RETURN_TYPE (OBJ_TYPE::*func)()const):
	WatcherObject(name),
	func_(func),
	This_(This)
	{
	}
	
	virtual ~WatcherMethodConst(){
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << id_ << type<RETURN_TYPE>() << (This_->*func_)();
	};

	void addToStream(MemoryStream* s)
	{
		(*s) << id_ << (This_->*func_)();
	};

	virtual void updateStream(MemoryStream* s){
		s_.clear(false);
		s_.append(s->data() + s->rpos(), sizeof(RETURN_TYPE));
		s->read_skip<RETURN_TYPE>();
	}

protected:
	FUNC func_;
	OBJ_TYPE* This_;
};

/*
	watcher管理器
*/
class Watchers: public Singleton<Watchers>
{
public:
	Watchers();
	~Watchers();

	void addToStream(MemoryStream* s);

	typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr< WatcherObject > > WATCHER_MAP;

	bool addWatcher(WatcherObject* pwo);
	bool delWatcher(std::string name);
	bool hasWatcher(std::string name);

	void updateStream(MemoryStream* s);
protected:
	WATCHER_MAP watcherObjs_;
};

/**
	用于监视一个值
	int32 a = 1;
	addWatcher("a", a);

	AAA a;
	a.x = 2;
	addWatcher("a", axxxx.x);
*/
template <class TYPE> 
inline void addWatcher(std::string name, TYPE type)
{
	Watchers::getSingleton().addWatcher(new WatcherValue<TYPE>(name, type));
};

/**
	用于监视一个函数的返回值

	int32 func(){}

	addWatcher("func", &func);
*/
template <class RETURN_TYPE> 
inline void addWatcher(std::string name, RETURN_TYPE (*func)())
{
	Watchers::getSingleton().addWatcher(new WatcherFunction<RETURN_TYPE>(name, func));
};

/**
	用于监视一个成员函数的返回值

	int32 AAA::func(){}
	AAA a;

	addWatcher("func", &a, &AAA::func);
*/
template <class RETURN_TYPE, class OBJ_TYPE> 
inline void addWatcher(std::string name, OBJ_TYPE* This, RETURN_TYPE (OBJ_TYPE::*func)())
{
	Watchers::getSingleton().addWatcher(new WatcherMethod<RETURN_TYPE, OBJ_TYPE>(name, This, func));
};

template <class RETURN_TYPE, class OBJ_TYPE> 
inline void addWatcher(std::string name, OBJ_TYPE* This, RETURN_TYPE (OBJ_TYPE::*func)()const)
{
	Watchers::getSingleton().addWatcher(new WatcherMethodConst<RETURN_TYPE, OBJ_TYPE>(name, This, func));
};

#ifdef ENABLE_WATCHERS
	#define WATCH_OBJECT addWatcher
#else
	inline void __addWatcher(...){}
	#define WATCH_OBJECT __addWatcher
#endif





}
#endif
