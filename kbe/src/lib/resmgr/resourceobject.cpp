// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "resmgr.h"
#include "resourceobject.h"
#include "common/timer.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
ResourceObject::ResourceObject(const char* res, uint32 flags):
resName_(res),
flags_(flags),
invalid_(false),
timeout_(0)
{
	update();
}

//-------------------------------------------------------------------------------------
ResourceObject::~ResourceObject()
{
	if(Resmgr::respool_timeout > 0)
	{
		DEBUG_MSG(fmt::format("ResourceObject::~ResourceObject(): {}\n", resName_));
	}
}

//-------------------------------------------------------------------------------------
void ResourceObject::update()
{
	timeout_ = timestamp() + uint64( Resmgr::respool_timeout * stampsPerSecond() );
}

//-------------------------------------------------------------------------------------
bool ResourceObject::valid() const
{
	return invalid_ || timestamp() < timeout_;
}

//-------------------------------------------------------------------------------------
FileObject::FileObject(const char* res, uint32 flags, const char* model):
ResourceObject(res, flags)
{
	fd_ = fopen(res, model);

	if(fd_ == NULL)
	{
		invalid_ = true;
		ERROR_MSG(fmt::format("FileObject::FileObject(): open({}) {} error!\n", model, resName_));
	}
}

//-------------------------------------------------------------------------------------
FileObject::~FileObject()
{
	fclose(fd_);
}

//-------------------------------------------------------------------------------------
bool FileObject::seek(uint32 idx, int flags)
{
	if(invalid_ || fd_ == NULL)
	{
		ERROR_MSG(fmt::format("FileObject::seek: {} invalid!\n", resName_));
		return false;
	}

	update();
	return fseek(fd_, idx, flags) != -1;
}

//-------------------------------------------------------------------------------------
uint32 FileObject::read(char* buf, uint32 limit)
{
	if(invalid_ || fd_ == NULL)
	{
		ERROR_MSG(fmt::format("FileObject::read: {} invalid!\n", resName_));
		return 0;
	}

	update();
	return fread(buf, sizeof(char), limit, fd_);
}

//-------------------------------------------------------------------------------------
uint32 FileObject::tell()
{
	if(invalid_ || fd_ == NULL)
	{
		ERROR_MSG(fmt::format("FileObject::tell: {} invalid!\n", resName_));
		return 0;
	}

	update();
	return ftell(fd_);
}

//-------------------------------------------------------------------------------------
}
