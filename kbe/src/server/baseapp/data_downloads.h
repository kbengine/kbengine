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

#ifndef KBE_DATA_DOWNLOADS_H
#define KBE_DATA_DOWNLOADS_H

#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class DataDownload;


class DataDownloads
{
public:
	DataDownloads();
	~DataDownloads();
	
	int16 pushDownload(DataDownload* pdl);

	void onDownloadCompleted(DataDownload* pdl);

	int16 freeID(int16 id);
private:
	std::map<int16, DataDownload*> downloads_;

	std::set< uint16 > usedIDs_;
};

class DataDownloadFactory
{
public:
	enum DataDownloadType
	{
		DATA_DOWNLOAD_STREAM_FILE = 1,
		DATA_DOWNLOAD_STREAM_STRING = 2
	};

	static DataDownload * create(DataDownloadType dltype, PyObjectPtr objptr, 
			const std::string & desc, int16 id);

};


}

#endif // KBE_DATA_DOWNLOADS_H
