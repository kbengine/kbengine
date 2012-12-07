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
	  name_(name)
	{
	}
	
	virtual ~WatcherObject(){
	}
	
	virtual void addToInitStream(MemoryStream* s) = 0;

	virtual void addToStream(MemoryStream* s) = 0;

	virtual void updateFromStream(MemoryStream* s) = 0;

	WATCHER_ID id(){ return id_; }
	void id(WATCHER_ID i){ id_ = i; }

	const char* name(){ return name_.c_str(); }

	virtual WATCHERTYPE type()const{ return WATCHER_TYPE_UNKNOWN; }
protected:
	std::string name_;
	WATCHER_ID id_;

};

/*
	watcher: 直接监视一个值
*/
template <typename T>
class WatcherValue : public WatcherObject
{
public:
	WatcherValue(std::string name, T* pVal):
	WatcherObject(name),
	pWatchVal_(pVal)
	{
	}
	
	virtual ~WatcherValue(){
		pWatchVal_ = NULL;
	}
	
	const T* pWatchVal()const{ 
		return pWatchVal_; 
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << id_ << type() << (*pWatchVal_);
	};

	void addToStream(MemoryStream* s){
		(*s) << id_ << (*pWatchVal_);
	};

	virtual void updateFromStream(MemoryStream* s){
		(*s) >> val_;
	}

	virtual WATCHERTYPE type()const{ return WATCHER_TYPE_UNKNOWN; }
protected:
	T* pWatchVal_;
	T val_;
};

template <>
WATCHERTYPE WatcherValue<uint8>::type()const
{ 
	return WATCHER_TYPE_UINT8; 
}

template <>
WATCHERTYPE WatcherValue<uint16>::type()const
{ 
	return WATCHER_TYPE_UINT16; 
}

template <>
WATCHERTYPE WatcherValue<uint32>::type()const
{ 
	return WATCHER_TYPE_UINT32; 
}

template <>
WATCHERTYPE WatcherValue<uint64>::type()const
{ 
	return WATCHER_TYPE_UINT64; 
}

template <>
WATCHERTYPE WatcherValue<int8>::type()const
{ 
	return WATCHER_TYPE_INT8; 
}

template <>
WATCHERTYPE WatcherValue<int16>::type()const
{ 
	return WATCHER_TYPE_INT16; 
}

template <>
WATCHERTYPE WatcherValue<int32>::type()const
{ 
	return WATCHER_TYPE_INT32; 
}

template <>
WATCHERTYPE WatcherValue<int64>::type()const
{ 
	return WATCHER_TYPE_INT64; 
}

template <>
WATCHERTYPE WatcherValue<float>::type()const
{ 
	return WATCHER_TYPE_FLOAT; 
}

template <>
WATCHERTYPE WatcherValue<double>::type()const
{ 
	return WATCHER_TYPE_DOUBLE; 
}

template <>
WATCHERTYPE WatcherValue<bool>::type()const
{ 
	return WATCHER_TYPE_BOOL; 
}

template <>
WATCHERTYPE WatcherValue<COMPONENT_TYPE>::type()const
{ 
	return WATCHER_TYPE_COMPONENT_TYPE; 
}

/*
template <>
WATCHERTYPE WatcherValue<char>::type()const
{ 
	return WATCHER_TYPE_CHAR; 
}
*/

template <>
WATCHERTYPE WatcherValue<char*>::type()const
{ 
	return WATCHER_TYPE_STRING; 
}

template <>
WATCHERTYPE WatcherValue<std::string>::type()const
{ 
	return WATCHER_TYPE_STRING; 
}

/*
	watcher: 监视一个方法返回的值
	typedef WatcherMethod<std::tr1::function<void(Base*, int64, bool)>> WatcherMethod_XX;
*/
template <typename RETURN_TYPE, typename BIND_METHOD>
class WatcherMethod : public WatcherObject
{
public:
	WatcherMethod(std::string name, BIND_METHOD bindmethod):
	WatcherObject(name),
	bindmethod_(bindmethod)
	{
	}
	
	virtual ~WatcherMethod(){
	}
	
	void addToInitStream(MemoryStream* s){
		(*s) << id_ << type() << bindmethod_();
	};

	void addToStream(MemoryStream* s)
	{
		(*s) << id_ << bindmethod_();
	};

	virtual void updateFromStream(MemoryStream* s){
		(*s) >> val_;
	}
protected:
	BIND_METHOD bindmethod_;
	RETURN_TYPE val_;
};

template <typename T>
inline WatcherObject* createWatcher(std::string name, T* pval)
{
	return new WatcherValue<T>(name, pval);
}

template <>
inline WatcherObject* createWatcher<uint8>(std::string name, uint8* pval)
{
	return new WatcherValue<uint8>(name, pval);
}

template <>
inline WatcherObject* createWatcher<uint16>(std::string name, uint16* pval)
{
	return createWatcher<uint16>(name, pval);
}

template <>
inline WatcherObject* createWatcher<uint32>(std::string name, uint32* pval)
{
	return createWatcher<uint32>(name, pval);
}

template <>
inline WatcherObject* createWatcher<uint64>(std::string name, uint64* pval)
{
	return createWatcher<uint64>(name, pval);
}

template <>
inline WatcherObject* createWatcher<int8>(std::string name, int8* pval)
{
	return createWatcher<int8>(name, pval);
}

template <>
inline WatcherObject* createWatcher<int16>(std::string name, int16* pval)
{
	return createWatcher<int16>(name, pval);
}

template <>
inline WatcherObject* createWatcher<int32>(std::string name, int32* pval)
{
	return new WatcherValue<int32>(name, pval);
}

template <>
inline WatcherObject* createWatcher<int64>(std::string name, int64* pval)
{
	return createWatcher<int64>(name, pval);
}

template <>
inline WatcherObject* createWatcher<bool>(std::string name, bool* pval)
{
	return createWatcher<bool>(name, pval);
}

/*
template <>
inline WatcherObject* createWatcher<char>(std::string name, char* pval)
{
	return createWatcher<char>(name, pval);
}
*/

template <>
inline WatcherObject* createWatcher<char*>(std::string name, char** pval)
{
	return createWatcher<char*>(name, pval);
}

template <>
inline WatcherObject* createWatcher<std::string>(std::string name, std::string* pval)
{
	return createWatcher<std::string>(name, pval);
}

template <>
inline WatcherObject* createWatcher<float>(std::string name, float* pval)
{
	return createWatcher<float>(name, pval);
}

template <>
inline WatcherObject* createWatcher<double>(std::string name, double* pval)
{
	return createWatcher<double>(name, pval);
}

template <>
inline WatcherObject* createWatcher<COMPONENT_TYPE>(std::string name, COMPONENT_TYPE* pval)
{
	return createWatcher<COMPONENT_TYPE>(name, pval);
}

template <class RETURN_TYPE, class BIND_METHOD>
class createMethodWatcher
{
public:
	static WatcherObject* create(std::string name){
		return new WatcherMethod<RETURN_TYPE, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<uint8, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<uint8, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<uint16, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<uint16, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<uint32, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<uint32, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<uint64, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<uint64, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<int8, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<int8, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<int16, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<int16, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<int32, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<int32, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<int64, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<int64, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<bool, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<uint8, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<char, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<char, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<char*, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<char*, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<std::string, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<std::string, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<float, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<float, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<double, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<double, BIND_METHOD>(name);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<COMPONENT_TYPE, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name){
		return create<int32, BIND_METHOD>(name);
	}
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

	void updateFromStream(MemoryStream* s);
protected:
	WATCHER_MAP watcherObjs_;
};

#define ADD_WATCH_VAL(NAME, VAL)															\
{																							\
}																							\


}
#endif
