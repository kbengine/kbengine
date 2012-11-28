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

		if(remainSent_ - currSent_ > GAME_PACKET_MAX_SIZE_TCP)
		{
			(*(*pBundle)).append(stream_ + currSent_, GAME_PACKET_MAX_SIZE_TCP);

			currSent_ += GAME_PACKET_MAX_SIZE_TCP;
			totalSentBytes_ += GAME_PACKET_MAX_SIZE_TCP;

			if(!send((*(*pBundle))))
			{
				DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), thread exit.\n") 
					% entityID() % id());

				return thread::TPTask::TPTASK_STATE_COMPLETED; 
			}
		}
		else
		{
			(*(*pBundle)).append(stream_ + currSent_, remainSent_ - currSent_);

			if(!send((*(*pBundle))))
			{
				DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), thread exit.\n") 
					% entityID() % id());

				return thread::TPTask::TPTASK_STATE_COMPLETED; 
			}

			totalSentBytes_ += remainSent_ - currSent_;
			currSent_ = remainSent_;
		}
	}

	if(totalSentBytes_ == totalBytes_)
	{
		pDataDownloads_->onDownloadCompleted(this);
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}
	
	DEBUG_MSG(boost::format("DataDownload::presentMainThread: proxy(%1%), downloadID(%2%), sentBytes=%3%/%4%.\n") % 
		entityID() % id() % totalSentBytes_ % this->totalBytes());

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
