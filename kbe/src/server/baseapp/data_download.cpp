// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "baseapp.h"
#include "data_download.h"
#include "data_downloads.h"
#include "resmgr/resmgr.h"

#include "client_lib/client_interface.h"

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
stream_(NULL),
entityID_(0),
error_(false)
{
}

//-------------------------------------------------------------------------------------
DataDownload::~DataDownload()
{
	SAFE_RELEASE_ARRAY(stream_);

	Proxy* proxy = static_cast<Proxy*>(Baseapp::getSingleton().findEntity(entityID_));

	if(proxy)
	{
		proxy->onStreamComplete(id_, totalBytes_ > 0 ? totalSentBytes_ == totalBytes_ : false);
	}
}

//-------------------------------------------------------------------------------------
bool DataDownload::send(const Network::MessageHandler& msgHandler, Network::Bundle* pBundle)
{
	Proxy* proxy = static_cast<Proxy*>(Baseapp::getSingleton().findEntity(entityID_));
	
	if(proxy && proxy->clientEntityCall())
	{
		proxy->sendToClient(msgHandler, pBundle);
	}
	else
	{
		Network::Bundle::reclaimPoolObject(pBundle);
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DataDownload::presentMainThread()
{
	if(error_)
	{
		ERROR_MSG(fmt::format("DataDownload::presentMainThread: proxy({}), downloadID({}), type({}), thread error.\n", 
			entityID(), id(), (int)type()));

		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}

	uint32 datasize = GAME_PACKET_MAX_SIZE_TCP - sizeof(int16) - sizeof(uint32);

	if ((remainSent_ > 0 && currSent_ < remainSent_) || totalBytes_ == 0)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

		if(!sentStart_)
		{
			pBundle->newMessage(ClientInterface::onStreamDataStarted);
			(*pBundle) << this->id();
			(*pBundle) << totalBytes_;
			(*pBundle) << descr_;
			(*pBundle) << type();

			sentStart_ = true;
			if(!send(ClientInterface::onStreamDataStarted, pBundle))
			{
				DEBUG_MSG(fmt::format("DataDownload::presentMainThread: proxy({}), downloadID({}), type({}), thread exit.\n",
					entityID(), id(), (int)type()));

				return thread::TPTask::TPTASK_STATE_COMPLETED; 
			}

			return thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD; 
		}

		pBundle->newMessage(ClientInterface::onStreamDataRecv);
		(*pBundle) << id();

		if(remainSent_ - currSent_ > datasize)
		{
			(*pBundle) << datasize;
			(*pBundle).append(getOutStream() + currSent_, datasize);

			currSent_ += datasize;
			totalSentBytes_ += datasize;

			if(!send(ClientInterface::onStreamDataRecv, pBundle))
			{
				DEBUG_MSG(fmt::format("DataDownload::presentMainThread: proxy({}), downloadID({}), type({}), thread exit.\n",
					entityID(), id(), (int)type()));

				error_ = true;
				return thread::TPTask::TPTASK_STATE_COMPLETED; 
			}
		}
		else
		{
			datasize = remainSent_ - currSent_;
			(*pBundle) << datasize;
			(*pBundle).append(getOutStream() + currSent_, datasize);

			if(!send(ClientInterface::onStreamDataRecv, pBundle))
			{
				DEBUG_MSG(fmt::format("DataDownload::presentMainThread: proxy({}), downloadID({}), type({}), thread exit.\n",
					entityID(), id(), (int)type()));

				error_ = true;
				return thread::TPTask::TPTASK_STATE_COMPLETED; 
			}

			totalSentBytes_ += datasize;
			currSent_ = remainSent_;
		}
	}

	if(totalSentBytes_ == totalBytes_)
	{
		DEBUG_MSG(fmt::format("DataDownload::presentMainThread: proxy({0}), downloadID({1}), type({6}), sentBytes={5},{2}/{3} ({4:.2f}%).\n",
			entityID(), id(), totalSentBytes_, this->totalBytes(), 100.0f, datasize, (int)type()));

		pDataDownloads_->onDownloadCompleted(this);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);


		pBundle->newMessage(ClientInterface::onStreamDataCompleted);
		(*pBundle) << this->id();

		send(ClientInterface::onStreamDataCompleted, pBundle);
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}
	
	DEBUG_MSG(fmt::format("DataDownload::presentMainThread: proxy({0}), downloadID({1}), type({6}), sentBytes={5},{2}/{3} ({4:.2f}%).\n",
		entityID(), id(), totalSentBytes_, this->totalBytes(), 
		(((float)totalSentBytes_ / (float)this->totalBytes()) * 100.0f), datasize, type()));

	if(currSent_ == remainSent_)
	{
		DEBUG_MSG(fmt::format("DataDownload::presentMainThread: proxy({}), downloadID({}), type({}), thread-continue.\n",
			entityID(), id(), (int)type()));

		return thread::TPTask::TPTASK_STATE_CONTINUE_CHILDTHREAD; 
	}

	return thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD; 
}

//-------------------------------------------------------------------------------------
StringDataDownload::StringDataDownload(PyObjectPtr objptr, 
							const std::string & descr, int16 id):
DataDownload(objptr, descr, id)
{
	PyObject* pyobj = PyUnicode_AsUTF8String(objptr.get());
	if(pyobj == NULL)
	{
		SCRIPT_ERROR_CHECK();
		error_ = true;
	}	
	else
	{
		totalBytes_ = (uint32)PyBytes_GET_SIZE(pyobj);
		stream_ = new char[totalBytes_ + 1];
		memcpy(getOutStream(), PyBytes_AS_STRING(pyobj), totalBytes_);
		Py_DECREF(pyobj);
	}
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
char* StringDataDownload::getOutStream()
{
	remainSent_ = totalBytes_ - totalSentBytes_;
	if(remainSent_ > 65535)
		remainSent_ = 65535;
	
	currSent_ = 0;
	return stream_ + totalSentBytes_;
}

//-------------------------------------------------------------------------------------
int8 StringDataDownload::type()
{
	return static_cast<int8>(DataDownloadFactory::DATA_DOWNLOAD_STREAM_STRING);
}

//-------------------------------------------------------------------------------------
FileDataDownload::FileDataDownload(PyObjectPtr objptr, 
							const std::string & descr, int16 id):
DataDownload(objptr, descr, id)
{
	path_ = PyUnicode_AsUTF8AndSize(objptr.get(), NULL);
	stream_ = new char[65537];
}

//-------------------------------------------------------------------------------------
FileDataDownload::~FileDataDownload()
{
}

//-------------------------------------------------------------------------------------
bool FileDataDownload::process()
{
	ResourceObjectPtr fptr = Resmgr::getSingleton().openResource(path_.c_str(), "rb");
	if(fptr == NULL || !fptr->valid())
	{
		ERROR_MSG(fmt::format("FileDataDownload::process(): can't open {}.\n", 
			Resmgr::getSingleton().matchRes(path_).c_str()));
		
		error_ = true;
		return false;
	}

	FileObject* f = static_cast<FileObject*>(fptr.get());
	if(totalBytes_ == 0)
	{
		f->seek(0, SEEK_END);
		totalBytes_ = f->tell();
	}

	f->seek(totalSentBytes_, SEEK_SET);
	remainSent_ = f->read(stream_, 65535);
	currSent_ = 0;

	if(remainSent_ == 0)
		remainSent_ = totalBytes_ - totalSentBytes_;

	return false;
}

//-------------------------------------------------------------------------------------
int8 FileDataDownload::type()
{
	return static_cast<int8>(DataDownloadFactory::DATA_DOWNLOAD_STREAM_FILE);
}


//-------------------------------------------------------------------------------------
}
