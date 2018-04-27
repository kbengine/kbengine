// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPT_PY_PROFILE_H
#define KBE_SCRIPT_PY_PROFILE_H

#include "common/common.h"
#include "common/smartpointer.h"
#include "scriptobject.h"

namespace KBEngine{ 
class MemoryStream;	
typedef SmartPointer<PyObject> PyObjectPtr;

namespace script{

class Script;

class PyProfile
{						
public:	
	/** 
		激活与停止某个profile
	*/
	static bool start(std::string profile);
	static bool stop(std::string profile);
	static bool dump(std::string profile, std::string fileName);
	static void addToStream(std::string profile, MemoryStream* s);
	static bool remove(std::string profile);

	static void print_stats(const std::string& sort = "time", const std::string& profileName = "kbengine");

	/** 
		初始化pickler 
	*/
	static bool initialize(Script* pScript);
	static void finalise(void);
	
private:
	typedef KBEUnordered_map< std::string, PyObjectPtr > PROFILES;
	static PROFILES profiles_;

	static PyObject* profileMethod_;

	static bool	isInit;										// 是否已经被初始化

	static Script* pScript_;
} ;

}
}
#endif // KBE_SCRIPT_PY_PROFILE_H
