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
DataDownloads::DataDownloads()
{
}

//-------------------------------------------------------------------------------------
DataDownloads::~DataDownloads()
{
}

//-------------------------------------------------------------------------------------
int16 DataDownloads::pushDownload(DataDownload* pdl)
{
	downloads_[pdl->id()] = pdl;

	pdl->pDataDownloads(this);

	Baseapp::getSingleton().threadPool().addTask(pdl);
	return pdl->id();
}

//-------------------------------------------------------------------------------------
void DataDownloads::onDownloadCompleted(DataDownload* pdl)
{
	INFO_MSG(boost::format("DataDownloads::onDownloadCompleted: proxy(%1%), downloadID(%2%) at %3%, sentTotalBytes=%4%.\n") % 
		pdl->entityID() % pdl->id() % pdl % pdl->totalBytes());

	downloads_.erase(pdl->id());
}

//-------------------------------------------------------------------------------------
}
