/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif
// common include	
#include "dataType.hpp"
#include "xmlplus/xmlplus.hpp"	
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>	
#include <map>	
#include <vector>
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
#define LIB_DLLAPI  __declspec(dllexport)

#ifdef __cplusplus  
extern "C" {  
#endif  

#ifdef __cplusplus  
}
#endif 

namespace KBEngine{

class DataTypes
{
protected:
	typedef std::map<std::string, DataType*> DATATYPE_MAP;
	static DATATYPE_MAP dataTypes_;
public:	
	DataTypes();
	virtual ~DataTypes();	

	static bool initialize(std::string file);
	static void finish(void);
	static bool addDateType(std::string name, DataType* dataType);
	static void delDataType(std::string name);
	static DataType* getDataType(std::string name);
	static DataType* getDataType(const char* name);
	static bool loadAlias(std::string& file);
};

}
#endif
