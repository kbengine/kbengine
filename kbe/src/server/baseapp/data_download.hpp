/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#ifndef __DATA_DOWNLOAD__
#define __DATA_DOWNLOAD__

#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "thread/threadtask.hpp"

namespace KBEngine{

class DataDownloads;

class DataDownload : public thread::TPTask
{
public:
	DataDownload(PyObjectPtr objptr, 
		const std::string & descr, int16 id);

	virtual ~DataDownload();
	
	virtual bool checkDescr(){ return true; }

	void pDataDownloads(DataDownloads* pDataDownloads){ pDataDownloads_ = pDataDownloads; }
	DataDownloads* pDataDownloads(){ return pDataDownloads_; }
protected:
	PyObjectPtr objptr_;
	std::string descr_;
	int16 id_;
	DataDownloads* pDataDownloads_;
};

class StringDataDownload : public DataDownload
{
public:
	StringDataDownload(PyObjectPtr objptr, 
		const std::string & descr, int16 id);

	virtual ~StringDataDownload();

	virtual bool process();
	virtual void presentMainThread();
};

class FileDataDownload : public DataDownload
{
public:
	FileDataDownload(PyObjectPtr objptr, 
		const std::string & descr, int16 id);

	virtual ~FileDataDownload();

	virtual bool process();
	virtual void presentMainThread();
};

}
#endif
