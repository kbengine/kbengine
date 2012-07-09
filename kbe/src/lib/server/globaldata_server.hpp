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
#ifndef __GLOBAL_DATA_SERVER_H__
#define __GLOBAL_DATA_SERVER_H__

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
	
namespace KBEngine{
namespace Mercury
{
	class Channel;
}

class GlobalDataServer
{	
public:	
	GlobalDataServer();
	~GlobalDataServer();
			
	/** 写数据 */
	bool write(Mercury::Channel* handler, const std::string& key, const std::string& value);
	
	/** 删除数据 */
	bool del(Mercury::Channel* handler, const std::string& key);	
	
	/** 添加该服务器所需要关心的组件类别 */
	void addConcernComponentType(COMPONENT_TYPE ct){ concernComponentTypes_.push_back(ct); }
	
	/** 广播一个数据的改变给所关心的组件 */
	void broadcastDataChange(Mercury::Channel* handler, const std::string& key, 
							const std::string& value, bool isDelete = false);
	
	/** 一个新的客户端登陆 */
	void onGlobalDataClientLogon(Mercury::Channel* client);
private:
	std::vector<COMPONENT_TYPE> concernComponentTypes_;						// 该GlobalDataServer所需要关心的组件类别
	typedef std::map<std::string, std::string> DATA_MAP;
	typedef DATA_MAP::iterator DATA_MAP_KEY;
	DATA_MAP dict_;
} ;

}
#endif