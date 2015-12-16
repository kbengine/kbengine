/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
