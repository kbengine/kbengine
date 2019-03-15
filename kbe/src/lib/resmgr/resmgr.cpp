// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "resmgr.h"
#include "helper/watcher.h"
#include "thread/threadguard.h"

#if KBE_PLATFORM != PLATFORM_WIN32
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#else
#include <tchar.h>
#include <direct.h>
#endif

namespace KBEngine{
KBE_SINGLETON_INIT(Resmgr);

uint64 Resmgr::respool_timeout = 0;
uint32 Resmgr::respool_buffersize = 0;
uint32 Resmgr::respool_checktick = 0;

//-------------------------------------------------------------------------------------
Resmgr::Resmgr():
kb_env_(),
respaths_(),
isInit_(false),
respool_(),
mutex_()
{
}

//-------------------------------------------------------------------------------------
Resmgr::~Resmgr()
{
	respool_.clear();
}

//-------------------------------------------------------------------------------------
bool Resmgr::initializeWatcher()
{
	WATCH_OBJECT("syspaths/KBE_ROOT", kb_env_.root_path);
	WATCH_OBJECT("syspaths/KBE_RES_PATH", kb_env_.res_path);
	WATCH_OBJECT("syspaths/KBE_BIN_PATH", kb_env_.bin_path);
	return true;
}

//-------------------------------------------------------------------------------------
void Resmgr::autoSetPaths()
{
	char path[MAX_BUF];
	char* ret = getcwd(path, MAX_BUF);
	if(ret == NULL)
		return;
	
	std::string s = path;
	size_t pos1;

	strutil::kbe_replace(s, "\\", "/");
	strutil::kbe_replace(s, "//", "/");
	pos1 = s.find("/kbe/bin/");

	if(pos1 == std::string::npos)
		return;

	s = s.substr(0, pos1 + 1);
	kb_env_.root_path = s;
	kb_env_.res_path = kb_env_.root_path + "kbe/res/;" + kb_env_.root_path + "/assets/;" + kb_env_.root_path + "/assets/scripts/;" + kb_env_.root_path + "/assets/res/";
}

//-------------------------------------------------------------------------------------
void Resmgr::updatePaths()
{
	char ch;
	
	if (kb_env_.root_path.size() > 0)
	{
		ch = kb_env_.root_path.at(kb_env_.root_path.size() - 1);
		if(ch != '/' && ch != '\\')
			kb_env_.root_path += "/";

		strutil::kbe_replace(kb_env_.root_path, "\\", "/");
		strutil::kbe_replace(kb_env_.root_path, "//", "/");
	}

	if(kb_env_.bin_path.size() > 0)
	{
		ch =  kb_env_.bin_path.at(kb_env_.bin_path.size() - 1);
		if(ch != '/' && ch != '\\')
			kb_env_.bin_path += "/";

		strutil::kbe_replace(kb_env_.bin_path, "\\", "/");
		strutil::kbe_replace(kb_env_.bin_path, "//", "/");
	}

	respaths_.clear();
	std::string tbuf = kb_env_.res_path;
	char splitFlag = ';';
	strutil::kbe_split<char>(tbuf, splitFlag, respaths_);

	// windows用户不能分割冒号， 可能会把盘符给分割了
#if KBE_PLATFORM != PLATFORM_WIN32
	if(respaths_.size() < 2)
	{
		respaths_.clear();
		splitFlag = ':';
		strutil::kbe_split<char>(tbuf, splitFlag, respaths_);
	}
#endif

	kb_env_.res_path = "";
	std::vector<std::string>::iterator iter = respaths_.begin();
	for(; iter != respaths_.end(); ++iter)
	{
		if((*iter).size() <= 0)
			continue;

		ch =  (*iter).at((*iter).size() - 1);
		if(ch != '/' && ch != '\\')
			(*iter) += "/";

		kb_env_.res_path += (*iter);
		kb_env_.res_path += splitFlag;
		strutil::kbe_replace(kb_env_.res_path, "\\", "/");
		strutil::kbe_replace(kb_env_.res_path, "//", "/");
	}

	if(kb_env_.res_path.size() > 0)
		kb_env_.res_path.erase(kb_env_.res_path.size() - 1);
}

//-------------------------------------------------------------------------------------
bool Resmgr::initialize()
{
	//if(isInit())
	//	return true;

	// 获取引擎环境配置
	kb_env_.root_path		= getenv("KBE_ROOT") == NULL ? "" : getenv("KBE_ROOT");
	kb_env_.res_path		= getenv("KBE_RES_PATH") == NULL ? "" : getenv("KBE_RES_PATH"); 
	kb_env_.bin_path		= getenv("KBE_BIN_PATH") == NULL ? "" : getenv("KBE_BIN_PATH"); 

	//kb_env_.root			= "/home/kbengine/";
	//kb_env_.res_path		= "/home/kbengine/kbe/res/;/home/kbengine/assets/;/home/kbengine/assets/scripts/;/home/kbengine/assets/res/"; 
	//kb_env_.bin_path		= "/home/kbengine/kbe/bin/server/"; 
	updatePaths();

	if (kb_env_.root_path == "" || kb_env_.res_path == "")
		autoSetPaths();

	updatePaths();

	if(getPySysResPath() == "" || getPyUserResPath() == "" || getPyUserScriptsPath() == "")
	{
		if (UNKNOWN_COMPONENT_TYPE != g_componentType && g_componentType != TOOL_TYPE)
		{
			printf("[ERROR] Resmgr::initialize: not set environment, (KBE_ROOT=%s, KBE_RES_PATH=%s, KBE_BIN_PATH=%s) invalid!\n",
				kb_env_.root_path.c_str(), kb_env_.res_path.c_str(), kb_env_.bin_path.c_str());
#if KBE_PLATFORM == PLATFORM_WIN32
			::MessageBox(0, L"Resmgr::initialize: not set environment, (KBE_ROOT, KBE_RES_PATH, KBE_BIN_PATH) invalid!\n", L"ERROR", MB_ICONERROR);
#endif
		}
	}

	isInit_ = true;

	respool_.clear();
	return true;
}

//-------------------------------------------------------------------------------------
void Resmgr::print(void)
{
	INFO_MSG(fmt::format("Resmgr::initialize: KBE_ROOT={0}\n", kb_env_.root_path));
	INFO_MSG(fmt::format("Resmgr::initialize: KBE_RES_PATH={0}\n", kb_env_.res_path));
	INFO_MSG(fmt::format("Resmgr::initialize: KBE_BIN_PATH={0}\n", kb_env_.bin_path));

#if KBE_PLATFORM == PLATFORM_WIN32
	printf("%s", fmt::format("KBE_ROOT = {0}\n", kb_env_.root_path).c_str());
	printf("%s", fmt::format("KBE_RES_PATH = {0}\n", kb_env_.res_path).c_str());
	printf("%s", fmt::format("KBE_BIN_PATH = {0}\n", kb_env_.bin_path).c_str());
	printf("\n");
#endif
}

//-------------------------------------------------------------------------------------
std::string Resmgr::matchRes(const std::string& res)
{
	return matchRes(res.c_str());
}

//-------------------------------------------------------------------------------------
std::string Resmgr::matchRes(const char* res)
{
	std::vector<std::string>::iterator iter = respaths_.begin();

	for(; iter != respaths_.end(); ++iter)
	{
		std::string fpath = ((*iter) + res);

		strutil::kbe_replace(fpath, "\\", "/");
		strutil::kbe_replace(fpath, "//", "/");

		if (access(fpath.c_str(), 0) == 0)
		{
			return fpath;
		}
	}

	return res;
}

//-------------------------------------------------------------------------------------
bool Resmgr::hasRes(const std::string& res)
{
	std::vector<std::string>::iterator iter = respaths_.begin();

	for(; iter != respaths_.end(); ++iter)
	{
		std::string fpath = ((*iter) + res);

		strutil::kbe_replace(fpath, "\\", "/");
		strutil::kbe_replace(fpath, "//", "/");

		if (access(fpath.c_str(), 0) == 0)
		{
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
FILE* Resmgr::openRes(std::string res, const char* mode)
{
	std::vector<std::string>::iterator iter = respaths_.begin();

	for(; iter != respaths_.end(); ++iter)
	{
		std::string fpath = ((*iter) + res);

		strutil::kbe_replace(fpath, "\\", "/");
		strutil::kbe_replace(fpath, "//", "/");

		FILE * f = fopen (fpath.c_str(), mode);
		if(f != NULL)
		{
			return f;
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
bool Resmgr::listPathRes(std::wstring path, const std::wstring& extendName, std::vector<std::wstring>& results)
{
	if(path.size() == 0)
	{
		ERROR_MSG("Resmgr::listPathRes: open dir [NULL] error!\n");
		return false;
	}

	if(path[path.size() - 1] != L'\\' && path[path.size() - 1] != L'/')
		path += L"/";

	std::vector<std::wstring> extendNames;
	strutil::kbe_split<wchar_t>(extendName, L'|', extendNames);

#if KBE_PLATFORM != PLATFORM_WIN32
	struct dirent *filename;
	DIR *dir;

	char* cpath = strutil::wchar2char(path.c_str());
	char pathstr[MAX_PATH];
	strcpy(pathstr, cpath);
	free(cpath);

	dir = opendir(pathstr);
	if (dir == NULL)
	{
		ERROR_MSG(fmt::format("Resmgr::listPathRes: open dir [{}] error!\n", pathstr));
		return false;
	}

	while ((filename = readdir(dir)) != NULL)
	{
		if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0)
			continue;

		struct stat s;
		char pathstrtmp[MAX_PATH * 2];
		sprintf(pathstrtmp, "%s%s", pathstr, filename->d_name);
		lstat(pathstrtmp, &s);

		if (S_ISDIR(s.st_mode))
		{
			wchar_t* wstr = strutil::char2wchar(pathstrtmp);
			listPathRes(wstr, extendName, results);
			free(wstr);
		}
		else
		{
			wchar_t* wstr = strutil::char2wchar(filename->d_name);

			if (extendName.size() == 0 || extendName == L"*" || extendName == L"*.*")
			{
				results.push_back(path + wstr);
			}
			else
			{
				if (extendNames.size() > 0)
				{
					std::vector<std::wstring> vec;
					strutil::kbe_split<wchar_t>(wstr, L'.', vec);

					for (size_t ext = 0; ext < extendNames.size(); ++ext)
					{
						if (extendNames[ext].size() > 0 && vec.size() > 1 && vec[vec.size() - 1] == extendNames[ext])
						{
							results.push_back(path + wstr);
						}
					}
				}
				else
				{
					results.push_back(path + wstr);
				}
			}

			free(wstr);
		}
	}

	closedir(dir);

#else
	wchar_t szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	wcscpy(szFind, path.c_str());
	wcscat(szFind, L"*");
	
	HANDLE hFind = FindFirstFile(szFind, &FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)
	{
		char* cstr = strutil::wchar2char(path.c_str());
		ERROR_MSG(fmt::format("Resmgr::listPathRes: open dir [{}] error!\n", cstr));
		free(cstr);
		return false;
	}

	while(TRUE)
	{
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(FindFileData.cFileName[0] != L'.')
			{
				wcscpy(szFind, path.c_str());
				wcscat(szFind, L"");
				wcscat(szFind, FindFileData.cFileName);
				listPathRes(szFind, extendName, results);
			}
		}
		else
		{
			if(extendName.size() == 0 || extendName == L"*" || extendName == L"*.*")
			{
				results.push_back(path + FindFileData.cFileName);
			}
			else
			{
				if(extendNames.size() > 0)
				{
					std::vector<std::wstring> vec;
					strutil::kbe_split<wchar_t>(FindFileData.cFileName, L'.', vec);

					for(size_t ext = 0; ext < extendNames.size(); ++ext)
					{
						if(extendNames[ext].size() > 0 && vec.size() > 1 && vec[vec.size() - 1] == extendNames[ext])
						{
							results.push_back(path + FindFileData.cFileName);
						}
					}
				}
				else
				{
					results.push_back(path + FindFileData.cFileName);
				}
			}
		}

		if(!FindNextFile(hFind, &FindFileData))
			break;
	}

	FindClose(hFind);

#endif

	return true;
}

//-------------------------------------------------------------------------------------
std::string Resmgr::matchPath(const std::string& path)
{
	return matchPath(path.c_str());
}

//-------------------------------------------------------------------------------------
std::string Resmgr::matchPath(const char* path)
{
	std::vector<std::string>::iterator iter = respaths_.begin();
	
	std::string npath = path;
	strutil::kbe_replace(npath, "\\", "/");
	strutil::kbe_replace(npath, "//", "/");

	for(; iter != respaths_.end(); ++iter)
	{
		std::string fpath = ((*iter) + npath);

		strutil::kbe_replace(fpath, "\\", "/");
		strutil::kbe_replace(fpath, "//", "/");
		
		if(!access(fpath.c_str(), 0))
		{
			return fpath;
		}
	}

	return "";
}

//-------------------------------------------------------------------------------------
std::string Resmgr::getPySysResPath()
{
	static std::string respath = "";

	if(respath == "")
	{
		respath = matchRes("server/kbengine_defaults.xml");
		std::vector<std::string> tmpvec;
		KBEngine::strutil::kbe_splits(respath, "server/kbengine_defaults.xml", tmpvec);

		if(tmpvec.size() > 1)
		{
			respath = tmpvec[0];
		}
		else
		{
			if(respaths_.size() > 0)
				respath = respaths_[0];
		}
	}

	return respath;
}

//-------------------------------------------------------------------------------------
std::string Resmgr::getPyUserResPath()
{
	static std::string respath = "";

	if(respath == "")
	{
		respath = matchRes("server/kbengine.xml");
		std::vector<std::string> tmpvec;
		KBEngine::strutil::kbe_splits(respath, "server/kbengine.xml", tmpvec);

		if(tmpvec.size() > 1)
		{
			respath = tmpvec[0];
		}
		else
		{
			if(respaths_.size() > 1)
				respath = respaths_[1];
			else if(respaths_.size() > 0)
				respath = respaths_[0];
		}
	}

	return respath;
}

//-------------------------------------------------------------------------------------
std::string Resmgr::getPyUserScriptsPath()
{
	static std::string path = "";

	if (path == "")
	{
		path = getPyUserResPath();

		std::string::size_type pos = path.rfind("res");
		path.erase(pos, path.size() - pos);
		path += "scripts/";
	}

	return path;
}

//-------------------------------------------------------------------------------------
std::string Resmgr::getPyUserComponentScriptsPath(COMPONENT_TYPE componentType)
{
	if (componentType == UNKNOWN_COMPONENT_TYPE)
	{
		static std::string path = "";

		if (path == "")
		{
			path = getPyUserScriptsPath();

			if (g_componentType == CELLAPP_TYPE)
				path += "cell/";
			else if (g_componentType == BASEAPP_TYPE)
				path += "base/";
			else if (g_componentType == BOTS_TYPE)
				path += "bots/";
			else if (g_componentType == CLIENT_TYPE)
				path += "client/";
			else
				KBE_ASSERT(false);
		}

		return path;
	}
	else
	{
		std::string path = "";

		if (path == "")
		{
			path = getPyUserScriptsPath();

			if (componentType == CELLAPP_TYPE)
				path += "cell/";
			else if (componentType == BASEAPP_TYPE)
				path += "base/";
			else if (componentType == BOTS_TYPE)
				path += "bots/";
			else if (componentType == CLIENT_TYPE)
				path += "client/";
			else
				KBE_ASSERT(false);
		}

		return path;
	}

	return "";
}

//-------------------------------------------------------------------------------------
std::string Resmgr::getPyUserAssetsPath()
{
	static std::string path = "";

	if (path == "")
	{
		path = getPyUserScriptsPath();
		strutil::kbe_replace(path, "/scripts", "");
		strutil::kbe_replace(path, "\\scripts", "");
	}

	return path;
}

//-------------------------------------------------------------------------------------
ResourceObjectPtr Resmgr::openResource(const char* res, const char* model, uint32 flags)
{
	std::string respath = matchRes(res);

	if(Resmgr::respool_checktick == 0)
	{
		return new FileObject(respath.c_str(), flags, model);
	}

	KBEngine::thread::ThreadGuard tg(&mutex_); 
	KBEUnordered_map< std::string, ResourceObjectPtr >::iterator iter = respool_.find(respath);
	if(iter == respool_.end())
	{
		FileObject* fobj = new FileObject(respath.c_str(), flags, model);
		respool_[respath] = fobj;
		fobj->update();
		return fobj;
	}

	iter->second->update();
	return iter->second;
}

//-------------------------------------------------------------------------------------
void Resmgr::update()
{
	KBEngine::thread::ThreadGuard tg(&mutex_); 
	KBEUnordered_map< std::string, ResourceObjectPtr >::iterator iter = respool_.begin();
	for(; iter != respool_.end();)
	{
		if(!iter->second->valid())
		{
			respool_.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
}

//-------------------------------------------------------------------------------------
void Resmgr::handleTimeout(TimerHandle handle, void * arg)
{
	update();
}

//-------------------------------------------------------------------------------------		
}
