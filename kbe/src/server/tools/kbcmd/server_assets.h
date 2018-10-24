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

#ifndef KBE_SERVER_ASSETS_H
#define KBE_SERVER_ASSETS_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "network/message_handler.h"

namespace KBEngine{

class ServerAssets
{
public:
	ServerAssets();
	virtual ~ServerAssets();

	virtual bool good() const;

	virtual bool create(const std::string& path);

	static ServerAssets* createServerAssets(const std::string& type);

	virtual std::string name() const {
		return "python_assets";
	}

	virtual bool copyAssetsSourceToPath(const std::string& path);

protected:
	std::string basepath_;

};

}
#endif
