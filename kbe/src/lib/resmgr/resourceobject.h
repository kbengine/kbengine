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

#ifndef KBE_RESOURCE_OBJECT_H
#define KBE_RESOURCE_OBJECT_H

#include "helper/debug_helper.h"
#include "common/common.h"
#include "common/smartpointer.h"	

namespace KBEngine{

class ResourceObject : public RefCountable
{
public:
	ResourceObject(const char* res, uint32 flags);
	virtual ~ResourceObject();
	
	const std::string& resName(){ return resName_; }
	uint32 flags() const{ return flags_; }

	bool valid() const;

	void update();

protected:
	std::string resName_;
	uint32 flags_;
	bool invalid_;

	uint64 timeout_;
};

class FileObject : public ResourceObject
{
public:
	FileObject(const char* res, uint32 flags, const char* model);
	virtual ~FileObject();
	
	FILE* fd(){ return fd_; }

	bool seek(uint32 idx, int flags = SEEK_SET);
	uint32 read(char* buf, uint32 limit);
	uint32 tell();

protected:
	FILE* fd_;
};

typedef SmartPointer<ResourceObject> ResourceObjectPtr;
}

#endif // KBE_RESOURCE_OBJECT_H
