#include "baseapp.hpp"
#include "data_download.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
DataDownload::DataDownload(PyObjectPtr objptr, 
						   const std::string & descr, int16 id):
objptr_(objptr),
descr_(descr),
id_(id)
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
}
