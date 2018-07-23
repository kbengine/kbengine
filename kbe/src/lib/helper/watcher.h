// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#ifndef KBE_HELPER_WATCHER_H
#define KBE_HELPER_WATCHER_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "common/smartpointer.h"
#include "common/singleton.h"
#include "common/memorystream.h"

namespace KBEngine
{

namespace Network{
}

typedef uint16 WATCHER_ID;
typedef uint8 WATCHER_VALUE_TYPE;

#define WATCHER_VALUE_TYPE_UNKNOWN				0
#define WATCHER_VALUE_TYPE_UINT8				1
#define WATCHER_VALUE_TYPE_UINT16				2
#define WATCHER_VALUE_TYPE_UINT32				3
#define WATCHER_VALUE_TYPE_UINT64				4
#define WATCHER_VALUE_TYPE_INT8					5
#define WATCHER_VALUE_TYPE_INT16				6
#define WATCHER_VALUE_TYPE_INT32				7
#define WATCHER_VALUE_TYPE_INT64				8
#define WATCHER_VALUE_TYPE_FLOAT				9
#define WATCHER_VALUE_TYPE_DOUBLE				10
#define WATCHER_VALUE_TYPE_CHAR					11
#define WATCHER_VALUE_TYPE_STRING				12
#define WATCHER_VALUE_TYPE_BOOL					13
#define WATCHER_VALUE_TYPE_COMPONENT_TYPE		14

class Watchers;

/*
	watcher基础对象
*/
class WatcherObject
{
public:
	WatcherObject(std::string path);
	
	virtual ~WatcherObject();

	virtual void addToInitStream(MemoryStream* s) {};

	virtual void addToStream(MemoryStream* s) {};

	WATCHER_ID id(){ return id_; }
	void id(WATCHER_ID i){ id_ = i; }

	const char* path(){ return path_.c_str(); }
	const char* name(){ return name_.c_str(); }

	template <class T>
	WATCHER_VALUE_TYPE type() const{ return WATCHER_VALUE_TYPE_UNKNOWN; }

	template <class T>
	void updateStream(MemoryStream* s){
		T v;
		(*s) >> v;
		strval_ = StringConv::val2str(v);
	}

	void addWitness(){ numWitness_++; }
	void delWitness(){ numWitness_--; }

	int32 numWitness() const{ return numWitness_; }

	const char* str() const{ return strval_.c_str(); }
	
	const char* getValue(){ return strval_.c_str(); }

	virtual WATCHER_VALUE_TYPE getType(){ return WATCHER_VALUE_TYPE_UNKNOWN; }

protected:
	std::string path_, name_, strval_;
	WATCHER_ID id_;
	int32 numWitness_;
};

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<uint8>() const
{
	return WATCHER_VALUE_TYPE_UINT8;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<uint16>() const
{
	return WATCHER_VALUE_TYPE_UINT16;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<uint32>() const
{
	return WATCHER_VALUE_TYPE_UINT32;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<uint64>() const
{
	return WATCHER_VALUE_TYPE_UINT64;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<int8>() const
{
	return WATCHER_VALUE_TYPE_INT8;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<int16>() const
{
	return WATCHER_VALUE_TYPE_INT16;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<int32>() const
{
	return WATCHER_VALUE_TYPE_INT32;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<int64>() const
{
	return WATCHER_VALUE_TYPE_INT64;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<float>() const
{
	return WATCHER_VALUE_TYPE_FLOAT;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<double>() const
{
	return WATCHER_VALUE_TYPE_DOUBLE;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<bool>() const
{
	return WATCHER_VALUE_TYPE_BOOL;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<char*>() const
{
	return WATCHER_VALUE_TYPE_STRING;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<const char*>() const
{
	return WATCHER_VALUE_TYPE_STRING;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<std::string>() const
{
	return WATCHER_VALUE_TYPE_STRING;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<std::string&>() const
{
	return WATCHER_VALUE_TYPE_STRING;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<const std::string&>() const
{
	return WATCHER_VALUE_TYPE_STRING;
}

template <>
inline WATCHER_VALUE_TYPE WatcherObject::type<COMPONENT_TYPE>() const
{
	return WATCHER_VALUE_TYPE_COMPONENT_TYPE;
}

template <>
inline void WatcherObject::updateStream<COMPONENT_TYPE>(MemoryStream* s)
{
	COMPONENT_TYPE v;
	(*s) >> v;
	strval_ = COMPONENT_NAME_EX(v);
}

template <>
inline void WatcherObject::updateStream<std::string>(MemoryStream* s)
{
	(*s) >> strval_;
}

template <>
inline void WatcherObject::updateStream<std::string&>(MemoryStream* s)
{
	updateStream<std::string>(s);
}

template <>
inline void WatcherObject::updateStream<const std::string&>(MemoryStream* s)
{
	updateStream<std::string>(s);
}

template <>
inline void WatcherObject::updateStream<char*>(MemoryStream* s)
{
	updateStream<std::string>(s);
}

template <>
inline void WatcherObject::updateStream<const char*>(MemoryStream* s)
{
	updateStream<std::string>(s);
}


/*
	watcher: 直接监视一个值
*/
template <class T>
class WatcherValue : public WatcherObject
{
public:
	WatcherValue(std::string path, const T& pVal):
	WatcherObject(path),
	watchVal_(pVal)
	{
	}
	
	virtual ~WatcherValue(){
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << path() << name() << id_ << type<T>() << watchVal_;
	};

	void addToStream(MemoryStream* s){
		(*s) << id_ << watchVal_;
	};

	T getValue(){ return watchVal_; }

	WATCHER_VALUE_TYPE getType(){ return type<T>(); }

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

	WatcherFunction(std::string path, RETURN_TYPE (*func)()):
	WatcherObject(path),
	func_(func)
	{
	}
	
	virtual ~WatcherFunction(){
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << path() << name() << id_ << type<RETURN_TYPE>() << (*func_)();
	};

	void addToStream(MemoryStream* s)
	{
		(*s) << id_ << (*func_)();
	};

	RETURN_TYPE getValue(){ return (*func_)(); }
	WATCHER_VALUE_TYPE getType(){ return type<RETURN_TYPE>(); }

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

	WatcherMethod(std::string path, OBJ_TYPE* obj, RETURN_TYPE (OBJ_TYPE::*func)()):
	WatcherObject(path),
	func_(func),
	obj_(obj)
	{
	}
	
	virtual ~WatcherMethod(){
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << path() << name() << id_ << type<RETURN_TYPE>() << (obj_->*func_)();
	};

	void addToStream(MemoryStream* s)
	{
		(*s) << id_ << (obj_->*func_)();
	};

	RETURN_TYPE getValue(){ return (obj_->*func_)(); }
	WATCHER_VALUE_TYPE getType(){ return type<RETURN_TYPE>(); }

protected:
	FUNC func_;
	OBJ_TYPE* obj_;
};

template <class RETURN_TYPE, class OBJ_TYPE>
class WatcherMethodConst : public WatcherObject
{
public:
	typedef RETURN_TYPE(OBJ_TYPE::*FUNC)() const;

	WatcherMethodConst(std::string path, OBJ_TYPE* obj, RETURN_TYPE (OBJ_TYPE::*func)() const):
	WatcherObject(path),
	func_(func),
	obj_(obj)
	{
	}
	
	virtual ~WatcherMethodConst(){
	}
	
	void addToInitStream(MemoryStream* s){
		RETURN_TYPE v = (obj_->*func_)();
		(*s) << path() << name() << id_ << type<RETURN_TYPE>() << v;
	};

	void addToStream(MemoryStream* s)
	{
		(*s) << id_ << (obj_->*func_)();
	};

	RETURN_TYPE getValue(){ return (obj_->*func_)(); }
	WATCHER_VALUE_TYPE getType(){ return type<RETURN_TYPE>(); }

protected:
	FUNC func_;
	OBJ_TYPE* obj_;
};

/*
	watcher管理器
*/
class Watchers
{
public:
	Watchers();
	~Watchers();
	
	void clear();

	static Watchers& rootWatchers();

	void addToStream(MemoryStream* s);

	void readWatchers(MemoryStream* s);
	typedef KBEUnordered_map<std::string, KBEShared_ptr< WatcherObject > > WATCHER_MAP;

	bool addWatcher(const std::string& path, WatcherObject* pwo);
	bool delWatcher(const std::string& name);
	bool hasWatcher(const std::string& name);

	KBEShared_ptr< WatcherObject > getWatcher(const std::string& name);

	void updateStream(MemoryStream* s);

	WATCHER_MAP& watcherObjs(){ return watcherObjs_; }

protected:
	WATCHER_MAP watcherObjs_;
};

class WatcherPaths
{
public:
	WatcherPaths();
	~WatcherPaths();

	void clear();

	static WatcherPaths& root();
	static bool finalise();

	void addToStream(MemoryStream* s);

	void readWatchers(std::string path, MemoryStream* s);
	void readChildPaths(std::string srcPath, std::string path, MemoryStream* s);
	void dirPath(std::string path, std::vector<std::string>& vec);

	typedef KBEUnordered_map<std::string, KBEShared_ptr<WatcherPaths> > WATCHER_PATHS;

	bool addWatcher(std::string path, WatcherObject* pwo);
	bool _addWatcher(std::string path, WatcherObject* pwo);

	WatcherObject* addWatcherFromStream(std::string path, std::string name, 
		WATCHER_ID wid, WATCHER_VALUE_TYPE wtype, MemoryStream* s);

	bool delWatcher(const std::string& fullpath);
	bool hasWatcher(const std::string& fullpath);
	KBEShared_ptr< WatcherObject > getWatcher(const std::string& fullpath);

	void updateStream(MemoryStream* s);

	Watchers& watchers(){ return watchers_; }
	WATCHER_PATHS& watcherPaths(){ return watcherPaths_; }

protected:
	WATCHER_PATHS watcherPaths_;
	Watchers watchers_;
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
inline WatcherObject* addWatcher(std::string path, const TYPE& type)
{
	path = std::string("root/") + path;
	WatcherValue<TYPE>* pwo = new WatcherValue<TYPE>(path, type);
	WatcherPaths::root().addWatcher(path, pwo);
	return pwo;
};

/**
	用于监视一个函数的返回值

	int32 func(){}

	addWatcher("func", &func);
*/
template <class RETURN_TYPE> 
inline WatcherObject* addWatcher(std::string path, RETURN_TYPE (*func)())
{
	path = std::string("root/") + path;
	WatcherFunction<RETURN_TYPE>* pwo = new WatcherFunction<RETURN_TYPE>(path, func);
	WatcherPaths::root().addWatcher(path, pwo);
	return pwo;
};

/**
	用于监视一个成员函数的返回值

	int32 AAA::func(){}
	AAA a;

	addWatcher("func", &a, &AAA::func);
*/
template <class RETURN_TYPE, class OBJ_TYPE> 
inline WatcherObject* addWatcher(std::string path, OBJ_TYPE* obj, RETURN_TYPE (OBJ_TYPE::*func)())
{
	path = std::string("root/") + path;
	WatcherMethod<RETURN_TYPE, OBJ_TYPE>* pwo = new WatcherMethod<RETURN_TYPE, OBJ_TYPE>(path, obj, func);
	WatcherPaths::root().addWatcher(path, pwo);
	return pwo;
};

template <class RETURN_TYPE, class OBJ_TYPE> 
inline WatcherObject* addWatcher(std::string path, OBJ_TYPE* obj, RETURN_TYPE (OBJ_TYPE::*func)() const)
{
	path = std::string("root/") + path;
	WatcherMethodConst<RETURN_TYPE, OBJ_TYPE>* pwo = new WatcherMethodConst<RETURN_TYPE, OBJ_TYPE>(path, obj, func);
	WatcherPaths::root().addWatcher(path, pwo);
	return pwo;
};

#ifdef ENABLE_WATCHERS
	#define WATCH_OBJECT addWatcher
	#define WATCH_FINALIZE WatcherPaths::root().clear();
#else
	inline WatcherObject* __addWatcher(...){ return NULL; }
	#define WATCH_OBJECT __addWatcher
	#define WATCH_FINALIZE
#endif


}
#endif
