/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __CSTDKBE__
#define __CSTDKBE__
#include "cstdkbe/platform.hpp"
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/kbeversion.hpp"
	
namespace KBEngine{
/** 安全的释放一个指针内存 */
#define SAFE_RELEASE(i)										\
	if (i)													\
		{													\
			delete i;										\
			i = NULL;										\
		}

/** 安全的释放一个指针数组内存 */
#define SAFE_RELEASE_ARRAY(i)								\
	if (i)													\
		{													\
			delete[] i;										\
			i = NULL;										\
		}

#ifdef CODE_INLINE
    #define INLINE    inline
#else
    #define INLINE
#endif

/** 定义服务器各组件类别 */
enum COMPONENT_TYPE
{
	UNKNOWN_COMPONENT_TYPE	= 0,
	DBMGR_TYPE		= 1,
	LOGINAPP_TYPE	= 2,
	BASEAPPMGR_TYPE	= 3,
	CELLAPPMGR_TYPE	= 4,
	CELLAPP_TYPE	= 5,
	BASEAPP_TYPE	= 6,
	CLIENT_TYPE		= 7,
	MACHINE_TYPE	= 8,
	CENTER_TYPE		= 9
};

/** 当前服务器组件类别 */
extern COMPONENT_TYPE g_componentType;
	
/** 定义服务器各组件名称 */
const char COMPONENT_NAME[][12] = {
	"unknown",
	"dbmgr",
	"loginapp",
	"baseappmgr",
	"cellappmgr",
	"cellapp",
	"baseapp",
	"client",
	"kbmachine",
	"kbcenter",
};
/** 一个cell边界能够看到另一个cell边界的最大范围 */
#define CELL_BORDER_WIDTH 500
	
/** 一个空间的一个chunk大小 */
#define SPACE_CHUNK_SIZE 100

/** 属性的lod广播级别范围的定义 */
#define DETAIL_LEVEL_NEAR													0	// lod级别：近						
#define DETAIL_LEVEL_MEDIUM													1	// lod级别：中
#define DETAIL_LEVEL_FAR													2	// lod级别：远	
#define DETAIL_LEVEL_UNKNOW													3	// lod级别：非常的远 (通常在这个级别内的entity不会对他广播任何属性（不包括位置方向等）)	
}
#endif // __CSTDKBE__