#include "baseapp.hpp"
#include "data_download.hpp"
#include "data_downloads.hpp"

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
	if(id > 0 && usedIDs_.find(id) != usedIDs_.end())
		return id;

	for(int16 i=1; i< 32767; i++)
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
	INFO_MSG(boost::format("DataDownloads::onDownloadCompleted: proxy(%1%), downloadID(%2%), type(%4%), sentTotalBytes=%3%.\n") % 
		pdl->entityID() % pdl->id() % pdl->totalBytes() % pdl->type());

	downloads_.erase(pdl->id());
	usedIDs_.erase(pdl->id());
}

//-------------------------------------------------------------------------------------
}
