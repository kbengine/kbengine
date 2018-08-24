// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "memorystream.h"
namespace KBEngine
{
static ObjectPool<MemoryStream> _g_objPool("MemoryStream");
//-------------------------------------------------------------------------------------
ObjectPool<MemoryStream>& MemoryStream::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
MemoryStream* MemoryStream::createPoolObject(const std::string& logPoint)
{
	return _g_objPool.createObject(logPoint);
}

//-------------------------------------------------------------------------------------
void MemoryStream::reclaimPoolObject(MemoryStream* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void MemoryStream::destroyObjPool()
{
	DEBUG_MSG(fmt::format("MemoryStream::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
MemoryStream::SmartPoolObjectPtr MemoryStream::createSmartPoolObj(const std::string& logPoint)
{
	return SmartPoolObjectPtr(new SmartPoolObject<MemoryStream>(ObjPool().createObject(logPoint), _g_objPool));
}

//-------------------------------------------------------------------------------------
size_t MemoryStream::getPoolObjectBytes()
{
	size_t bytes = sizeof(rpos_) + sizeof(wpos_) + data_.capacity();
	return bytes;
}

//-------------------------------------------------------------------------------------
void MemoryStream::onReclaimObject()
{
	if(data_.capacity() > DEFAULT_SIZE * 2)
		data_.reserve(DEFAULT_SIZE);

	clear(false);
}

//-------------------------------------------------------------------------------------
MemoryStream::~MemoryStream()
{
	clear(true);
}

//-------------------------------------------------------------------------------------
} 


