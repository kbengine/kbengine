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
	downloads_.push_back(std::tr1::shared_ptr<DataDownload>(pdl));

	return 0;
}

//-------------------------------------------------------------------------------------
}
