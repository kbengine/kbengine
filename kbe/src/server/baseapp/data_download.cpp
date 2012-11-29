#include "baseapp.hpp"
#include "data_download.hpp"
#include "resmgr/resmgr.hpp"

#include "client_lib/client_interface.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
DataDownload::DataDownload(PyObjectPtr objptr, 
						   const std::string & descr, int16 id):
objptr_(objptr),
descr_(descr),
id_(id),
pDataDownloads_(NULL),
sentStart_(false),
totalBytes_(0),
totalSentBytes_(0),
remainSent_(0),
currSent_(0),
fini_(false),
entityID_(0)
{
}

//-------------------------------------------------------------------------------------
DataDownload::~DataDownload()
{
}

//-------------------------------------------------------------------------------------
bool DataDownload::send(Mercury::Bundle& bundle)
{
	Proxy* proxy = static_cast<Proxy*>(Baseapp::getSingleton().findEntity(entityID_));
	
	if(proxy){
		proxy->getClientMailbox()->postMail(bundle);
	}
	else{
		return false;
	}

	return true;
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
thread::TPTask::TPTaskState StringDataDownload::presentMainThread()
{
	if(fini_)
		thread::TPTask::TPTASK_STATE_COMPLETED; 

	return thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD; 
}

//-------------------------------------------------------------------------------------
FileDataDownload::FileDataDownload(PyObjectPtr objptr, 
							const std::string & descr, int16 id):
DataDownload(objptr, descr, id)
{
	wchar_t* PyUnicode_AsWideCharStringRet1 = PyUnicode_AsWideCharString(objptr.get(), NULL);
	char* pDescr = wchar2char(PyUnicode_AsWideCharStringRet1);
	PyMem_Free(PyUnicode_AsWideCharStringRet1);

	path_ = pDescr;
	free(pDescr);
}

//-------------------------------------------------------------------------------------
FileDataDownload::~FileDataDownload()
{
}

//-------------------------------------------------------------------------------------
bool FileDataDownload::process()
{
	FILE* f = Resmgr::getSingleton().openResource(path_.c_str(), "rb");
	if(f == NULL)
	{
		ERROR_MSG(boost::format("FileDataDownload::process(): can't open %1%.\n") % 
			Resmgr::getSingleton().matchRes(path_).c_str());

		return false;
	}

	if(totalBytes_ == 0)
	{
		fseek(f, 0, SEEK_END);
		totalBytes_ = ftell(f);
	}

	fseek(f, totalSentBytes_, SEEK_SET);
	remainSent_ = fread(stream_, sizeof(char), 65535, f);
	currSent_ = 0;

	if(remainSent_ == 0)
		remainSent_ = totalBytes_ - totalSentBytes_;

	fclose(f);
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState FileDataDownload::presentMainThread()
{
	uint32 datasize = GAME_PACKET_MAX_SIZE_TCP - sizeof(int16) - sizeof(uint32);

	if(remainSent_ > 0 && currSent_ < remainSent_)
	{
		Mercury::Bundle::SmartPoolObjectPtr pBundle = Mercury::Bundle::createSmartPoolObj();

		if(!sentStart_)
		{
			(*pBundle)->newMessage(ClientInterface::onStreamDataStarted);
			(*(*pBundle)) << this->id();
			(*(*pBundle)) << totalBytes_;
			(*(*pBundle)) << descr_;

			sentStart_ = true;
			if(!send((*(*pBundle))))
			{
				DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), thread exit.\n") 
					% entityID() % id());
				return thread::TPTask::TPTASK_STATE_COMPLETED; 
			}

			return thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD; 
		}

		(*pBundle)->newMessage(ClientInterface::onStreamDataRecv);
		(*(*pBundle)) << id();

		if(remainSent_ - currSent_ > datasize)
		{
			(*(*pBundle)) << datasize;
			(*(*pBundle)).append(stream_ + currSent_, datasize);

			currSent_ += datasize;
			totalSentBytes_ += datasize;

			if(!send((*(*pBundle))))
			{
				DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), thread exit.\n") 
					% entityID() % id());

				return thread::TPTask::TPTASK_STATE_COMPLETED; 
			}
		}
		else
		{
			datasize = remainSent_ - currSent_;
			(*(*pBundle)) << datasize;
			(*(*pBundle)).append(stream_ + currSent_, datasize);

			if(!send((*(*pBundle))))
			{
				DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), thread exit.\n") 
					% entityID() % id());

				return thread::TPTask::TPTASK_STATE_COMPLETED; 
			}

			totalSentBytes_ += datasize;
			currSent_ = remainSent_;
		}
	}

	if(totalSentBytes_ == totalBytes_)
	{
		DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), sentBytes=%6%,%3%/%4% (%5$.2f%%).\n") % 
			entityID() % id() % totalSentBytes_ % this->totalBytes() % 100.0f % datasize);
		pDataDownloads_->onDownloadCompleted(this);
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}
	
	DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), sentBytes=%6%,%3%/%4% (%5$.2f%%).\n") % 
		entityID() % id() % totalSentBytes_ % this->totalBytes() % (((float)totalSentBytes_ / (float)this->totalBytes()) * 100.0f) % datasize);

	if(currSent_ == remainSent_)
	{
		DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), thread-continue.\n") 
			% entityID() % id());

		return thread::TPTask::TPTASK_STATE_CONTINUE_CHILDTHREAD; 
	}

	return thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD; 
}

//-------------------------------------------------------------------------------------
}
