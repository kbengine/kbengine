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

#include "resmgr.hpp"

namespace KBEngine{
KBE_SINGLETON_INIT(Resmgr);

Resmgr g_resMgr;

//-------------------------------------------------------------------------------------
Resmgr::Resmgr()
{
}

//-------------------------------------------------------------------------------------
Resmgr::~Resmgr()
{
}

//-------------------------------------------------------------------------------------
bool Resmgr::initialize()
{
	// 获取引擎环境配置
	kb_env_.root			= getenv("KBE_ROOT") == NULL ? "" : getenv("KBE_ROOT");
	kb_env_.res_path		= getenv("KBE_RES_PATH") == NULL ? "" : getenv("KBE_RES_PATH"); 
	kb_env_.hybrid_path		= getenv("KBE_HYBRID_PATH") == NULL ? "" : getenv("KBE_HYBRID_PATH"); 

	kb_env_.root				= "D:/kbengine/";
	kb_env_.res_path			= "D:/kbengine/kbe/res/;D:/kbengine/demo/;D:/kbengine/demo/res/"; 
	kb_env_.hybrid_path		= "D:/kbengine/kbe/bin/Hybrid/"; 

	std::string tbuf = kb_env_.res_path;
	kbe_split<char>(tbuf, ';', respaths_);
	return true;
}

//-------------------------------------------------------------------------------------
void Resmgr::pirnt(void)
{
	INFO_MSG("Resmgr::initialize: KBE_ROOT=%s\n", kb_env_.root.c_str());
	INFO_MSG("Resmgr::initialize: KBE_RES_PATH=%s\n", kb_env_.res_path.c_str());
	INFO_MSG("Resmgr::initialize: KBE_HYBRID_PATH=%s\n", kb_env_.hybrid_path.c_str());
}

//-------------------------------------------------------------------------------------
std::string Resmgr::matchRes(std::string path)
{
	std::vector<std::string>::iterator iter = respaths_.begin();

	for(; iter != respaths_.end(); iter++)
	{
		std::string fpath = ((*iter) + path);
		FILE * f = fopen (fpath.c_str(), "r");
		if(f != NULL)
		{
			fclose(f);
			return fpath;
		}
	}
	return "";
}


//-------------------------------------------------------------------------------------		
}
