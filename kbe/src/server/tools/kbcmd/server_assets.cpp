// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "kbcmd.h"
#include "server_assets.h"
#include "entitydef/entitydef.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/property.h"
#include "entitydef/method.h"
#include "entitydef/datatypes.h"
#include "entitydef/datatype.h"
#include "resmgr/resmgr.h"
#include "server/common.h"
#include "server/serverconfig.h"
#include "common/kbeversion.h"
#include "network/fixed_messages.h"

#include "client_lib/client_interface.h"
#include "baseapp/baseapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "dbmgr/dbmgr_interface.h"
#include "loginapp/loginapp_interface.h"

namespace KBEngine {	

//-------------------------------------------------------------------------------------
ServerAssets::ServerAssets():
	basepath_()
{

}

//-------------------------------------------------------------------------------------
ServerAssets::~ServerAssets()
{

}

//-------------------------------------------------------------------------------------
ServerAssets* ServerAssets::createServerAssets(const std::string& type)
{
	//std::string lowerType = type;
	//std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), tolower);

	return new ServerAssets();
}

//-------------------------------------------------------------------------------------
bool ServerAssets::good() const
{
	return true;
}

//-------------------------------------------------------------------------------------
bool ServerAssets::create(const std::string& path)
{
	basepath_ = path;

	if (basepath_[basepath_.size() - 1] != '\\' && basepath_[basepath_.size() - 1] != '/')
		basepath_ += "/";

	std::string findpath = "sdk_templates/server/" + name();

	std::string getpath = Resmgr::getSingleton().matchPath(findpath);

	if (getpath.size() == 0 || findpath == getpath)
	{
		ERROR_MSG(fmt::format("ServerAssets::create(): not found path({})\n",
			findpath));

		return false;
	}

	if (!copyAssetsSourceToPath(getpath))
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool ServerAssets::copyAssetsSourceToPath(const std::string& path)
{
	wchar_t* wpath = strutil::char2wchar(path.c_str());
	std::wstring sourcePath = wpath;
	free(wpath);

	wpath = strutil::char2wchar(basepath_.c_str());
	std::wstring destPath = wpath;
	free(wpath);

	std::vector<std::wstring> results;
	if (!Resmgr::getSingleton().listPathRes(sourcePath, L"*", results))
		return false;

	wchar_t* wfindpath = strutil::char2wchar(std::string("sdk_templates/server/" + name()).c_str());
	std::wstring findpath = wfindpath;
	free(wfindpath);

	std::vector<std::wstring>::iterator iter = results.begin();
	for (; iter != results.end(); ++iter)
	{
		std::wstring::size_type fpos = (*iter).find(findpath);

		char* ccattr = strutil::wchar2char((*iter).c_str());
		std::string currpath = ccattr;
		free(ccattr);

		if (fpos == std::wstring::npos)
		{
			ERROR_MSG(fmt::format("ServerAssets::copyAssetsSourceToPath(): split path({}) error!\n",
				currpath));

			return false;
		}

		std::wstring targetFile = (*iter);
		targetFile.erase(0, fpos + findpath.size() + 1);
		targetFile = (destPath + targetFile);

		std::wstring basepath = targetFile;
		fpos = targetFile.rfind(L"/");

		ccattr = strutil::wchar2char(targetFile.c_str());
		std::string currTargetFile = ccattr;
		free(ccattr);

		if (fpos == std::wstring::npos)
		{
			ERROR_MSG(fmt::format("ServerAssets::copyAssetsSourceToPath(): split basepath({}) error!\n",
				currTargetFile));

			return false;
		}

		basepath.erase(fpos, basepath.size() - fpos);

		ccattr = strutil::wchar2char(basepath.c_str());
		std::string currbasepath = ccattr;
		free(ccattr);

		if (KBCMD::creatDir(currbasepath.c_str()) == -1)
		{
			ERROR_MSG(fmt::format("ServerAssets::copyAssetsSourceToPath(): creating directory error! path={}\n", currbasepath));
			return false;
		}

		std::ifstream input(currpath.c_str(), std::ios::binary);
		std::ofstream output(currTargetFile.c_str(), std::ios::binary);

		std::stringstream ss;
		std::string filebody;

		ss << input.rdbuf();
		filebody = ss.str();

		output << filebody;

		output.close();
		input.close();
	}

	return true;
}

//-------------------------------------------------------------------------------------
}
