#include "baseapp.hpp"
#include "data_download.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
DataDownload::DataDownload(PyObjectPtr objptr, 
						   const std::string & descr, int16 id):
objptr_(objptr),
descr_(descr),
id_(id),
pDataDownloads_(NULL)
{
}

//-------------------------------------------------------------------------------------
DataDownload::~DataDownload()
{
}

//-------------------------------------------------------------------------------------
StringDataDownload::StringDataDownload(PyObjectPtr objptr, 
							const std::string & descr, int16 id):
DataDownload(objptr, descr, id)
{
}

//-------------------------------------------------------------------------------------
StringDataDownload::~StringDataDownload()
{
}

//-------------------------------------------------------------------------------------
bool StringDataDownload::process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void StringDataDownload::presentMainThread()
{
}

//-------------------------------------------------------------------------------------
FileDataDownload::FileDataDownload(PyObjectPtr objptr, 
							const std::string & descr, int16 id):
DataDownload(objptr, descr, id)
{
}

//-------------------------------------------------------------------------------------
FileDataDownload::~FileDataDownload()
{
}

//-------------------------------------------------------------------------------------
bool FileDataDownload::process()
{
	return false;
}

//-------------------------------------------------------------------------------------
void FileDataDownload::presentMainThread()
{
}

//-------------------------------------------------------------------------------------
}
