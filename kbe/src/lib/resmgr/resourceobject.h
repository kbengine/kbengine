// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
