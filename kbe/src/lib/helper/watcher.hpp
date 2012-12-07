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
	WatcherObject(std::string name, WATCHERTYPE type = WATCHER_TYPE_UNKNOWN):
	  name_(name),
	  type_(type)
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

	virtual WATCHERTYPE type()const{ return type_; }
	virtual void type(WATCHERTYPE t){ type_ = t; }
protected:
	std::string name_;
	WATCHER_ID id_;
	WATCHERTYPE type_;

};

/*
	watcher: 直接监视一个值
*/
template <typename T>
class WatcherValue : public WatcherObject
{
public:
	WatcherValue(std::string name, T* pVal, WATCHERTYPE type):
	WatcherObject(name, type),
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

protected:
	T* pWatchVal_;
	T val_;
};

/*
	watcher: 监视一个方法返回的值
	typedef WatcherMethod<std::tr1::function<void(Base*, int64, bool)>> WatcherMethod_XX;
*/
template <typename RETURN_TYPE, typename BIND_METHOD>
class WatcherMethod : public WatcherObject
{
public:
	WatcherMethod(std::string name, BIND_METHOD bindmethod, WATCHERTYPE type):
	WatcherObject(name, type),
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
	return new WatcherValue<T>(name, pval, WATCHER_TYPE_UNKOWN);
}

template <>
inline WatcherObject* createWatcher<uint8>(std::string name, uint8* pval)
{
	return new WatcherValue<uint8>(name, pval, WATCHER_TYPE_UINT8);
}

template <>
inline WatcherObject* createWatcher<uint16>(std::string name, uint16* pval)
{
	return new WatcherValue<uint16>(name, pval, WATCHER_TYPE_UINT16);
}

template <>
inline WatcherObject* createWatcher<uint32>(std::string name, uint32* pval)
{
	return new WatcherValue<uint32>(name, pval, WATCHER_TYPE_UINT32);
}

template <>
inline WatcherObject* createWatcher<uint64>(std::string name, uint64* pval)
{
	return new WatcherValue<uint64>(name, pval, WATCHER_TYPE_UINT64);
}

template <>
inline WatcherObject* createWatcher<int8>(std::string name, int8* pval)
{
	return new WatcherValue<int8>(name, pval, WATCHER_TYPE_INT8);
}

template <>
inline WatcherObject* createWatcher<int16>(std::string name, int16* pval)
{
	return new WatcherValue<int16>(name, pval, WATCHER_TYPE_INT16);
}

template <>
inline WatcherObject* createWatcher<int32>(std::string name, int32* pval)
{
	return new WatcherValue<int32>(name, pval, WATCHER_TYPE_INT32);
}

template <>
inline WatcherObject* createWatcher<int64>(std::string name, int64* pval)
{
	return new WatcherValue<int64>(name, pval, WATCHER_TYPE_INT64);
}

template <>
inline WatcherObject* createWatcher<bool>(std::string name, bool* pval)
{
	return new WatcherValue<bool>(name, pval, WATCHER_TYPE_BOOL);
}

template <>
inline WatcherObject* createWatcher<char*>(std::string name, char** pval)
{
	return new WatcherValue<char*>(name, pval, WATCHER_TYPE_STRING);
}

template <>
inline WatcherObject* createWatcher<std::string>(std::string name, std::string* pval)
{
	return new WatcherValue<std::string>(name, pval, WATCHER_TYPE_STRING);
}

template <>
inline WatcherObject* createWatcher<float>(std::string name, float* pval)
{
	return new WatcherValue<float>(name, pval, WATCHER_TYPE_FLOAT);
}

template <>
inline WatcherObject* createWatcher<double>(std::string name, double* pval)
{
	return new WatcherValue<double>(name, pval, WATCHER_TYPE_DOUBLE);
}

template <>
inline WatcherObject* createWatcher<COMPONENT_TYPE>(std::string name, COMPONENT_TYPE* pval)
{
	return new WatcherValue<COMPONENT_TYPE>(name, pval, WATCHER_TYPE_COMPONENT_TYPE);
}

template <class RETURN_TYPE, class BIND_METHOD>
class createMethodWatcher
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<RETURN_TYPE, BIND_METHOD>(name, method, WATCHER_TYPE_UNKOWN);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<uint8, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<uint8, BIND_METHOD>(name, method, WATCHER_TYPE_UINT8);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<uint16, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<uint16, BIND_METHOD>(name, method, WATCHER_TYPE_UINT16);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<uint32, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<uint32, BIND_METHOD>(name, method, WATCHER_TYPE_UINT32);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<uint64, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<uint64, BIND_METHOD>(name, method, WATCHER_TYPE_UINT64);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<int8, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<int8, BIND_METHOD>(name, method, WATCHER_TYPE_INT8);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<int16, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<int16, BIND_METHOD>(name, method, WATCHER_TYPE_INT16);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<int32, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<int32, BIND_METHOD>(name, method, WATCHER_TYPE_INT32);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<int64, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<int64, BIND_METHOD>(name, method, WATCHER_TYPE_INT64);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<bool, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<bool, BIND_METHOD>(name, method, WATCHER_TYPE_BOOL);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<char*, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<char*, BIND_METHOD>(name, method, WATCHER_TYPE_STRING);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<std::string, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<std::string, BIND_METHOD>(name, method, WATCHER_TYPE_STRING);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<float, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<float, BIND_METHOD>(name, method, WATCHER_TYPE_FLOAT);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<double, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<double, BIND_METHOD>(name, method, WATCHER_TYPE_DOUBLE);
	}
};

template <class BIND_METHOD>
class createMethodWatcher<COMPONENT_TYPE, BIND_METHOD>
{
public:
	static WatcherObject* create(std::string name, BIND_METHOD method){
		return new WatcherMethod<COMPONENT_TYPE, BIND_METHOD>(name, method, WATCHER_TYPE_COMPONENT_TYPE);
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

#define ADD_WATCH(NAME, OBJTYPE)															\
{																							\
	WatcherObject* obj = createWatcher(NAME, OBJTYPE);										\
	Watchers::getSingleton().addWatcher(obj);												\
}																							\

#define ADD_WATCH_METHOD(NAME, OBJTYPE, RETURN_TYPE)										\
{																							\
}																							\


}
#endif
