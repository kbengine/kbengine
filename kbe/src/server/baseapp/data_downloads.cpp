/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#include "baseapp.h"
#include "data_download.h"
#include "data_downloads.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
DataDownload * DataDownloadFactory::create(DataDownloadType dltype, PyObjectPtr objptr, 
		const std::string & desc, int16 id)
{
	switch(dltype)
	{
		case DATA_DOWNLOAD_STREAM_STRING:
			return new StringDataDownload(objptr, desc, id);
		case DATA_DOWNLOAD_STREAM_FILE:
			return new FileDataDownload(objptr, desc, id);
		default:
			break;
	};

	return NULL;
}
	
//-------------------------------------------------------------------------------------
DataDownloads::DataDownloads():
downloads_(),
usedIDs_()
{
}

//-------------------------------------------------------------------------------------
DataDownloads::~DataDownloads()
{
}

//-------------------------------------------------------------------------------------
int16 DataDownloads::freeID(int16 id)
{
	if(id > 0 && usedIDs_.find(id) == usedIDs_.end())
		return id;

	for(int16 i=2; i< 32767; ++i)
	{
		if(usedIDs_.find(id + i) == usedIDs_.end())
			return id + i;
	}

	return id;
}

//-------------------------------------------------------------------------------------
int16 DataDownloads::pushDownload(DataDownload* pdl)
{
	pdl->id(freeID(pdl->id()));
	usedIDs_.insert(pdl->id());
	downloads_[pdl->id()] = pdl;

	pdl->pDataDownloads(this);

	Baseapp::getSingleton().threadPool().addTask(pdl);
	return pdl->id();
}

//-------------------------------------------------------------------------------------
void DataDownloads::onDownloadCompleted(DataDownload* pdl)
{
	INFO_MSG(fmt::format("DataDownloads::onDownloadCompleted: proxy({0}), downloadID({1}), type({3}), sentTotalBytes={2}.\n", 
		pdl->entityID(), pdl->id(), pdl->totalBytes(), (int)pdl->type()));

	downloads_.erase(pdl->id());
	usedIDs_.erase(pdl->id());
}

//-------------------------------------------------------------------------------------
}
