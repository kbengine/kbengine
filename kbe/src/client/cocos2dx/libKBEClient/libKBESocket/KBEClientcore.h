#pragma once

//#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
//#  define KBE_PLATFORM PLATFORM_WIN32
//#elif defined( __INTEL_COMPILER )
//#  define KBE_PLATFORM PLATFORM_INTEL
//#elif defined( __APPLE_CC__ )
//#  define KBE_PLATFORM PLATFORM_APPLE
//#else
//#  define KBE_PLATFORM PLATFORM_UNIX
//#endif
//
//#define COMPILER_MICROSOFT 0
//#define COMPILER_GNU	   1
//#define COMPILER_BORLAND   2
//#define COMPILER_INTEL     3
//
//#ifdef _MSC_VER
//#  define KBE_COMPILER COMPILER_MICROSOFT
//#elif defined( __INTEL_COMPILER )
//#  define KBE_COMPILER COMPILER_INTEL
//#elif defined( __BORLANDC__ )
//#  define KBE_COMPILER COMPILER_BORLAND
//#elif defined( __GNUC__ )
//#  define KBE_COMPILER COMPILER_GNU
//#else
//#  pragma error "FATAL ERROR: Unknown compiler."
//#endif
////
//#ifdef _WIN32
//#define WIN32_LEAN_AND_MEAN
////http://www.cnblogs.com/tonyyang132/archive/2009/10/14/1583110.html
//// LEAN_AND_MEAN can avoid winsock issue. e.g:  error C2011: 'sockaddr' : 'struct' type redefinition
//#include <Windows.h>
//#endif
//
//* Use correct types for x64 platforms, too */

//#if KBE_COMPILER != COMPILER_GNU
//typedef signed __int64											int64;
//typedef signed __int32											int32;
//typedef signed __int16											int16;
//typedef signed __int8											int8;
//typedef unsigned __int64										uint64;
//typedef unsigned __int32										uint32;
//typedef unsigned __int16										uint16;
//typedef unsigned __int8											uint8;
//typedef INT_PTR													intptr;
//typedef UINT_PTR        										uintptr;
//#define PRI64													"lld"
//#define PRIu64													"llu"
//#define PRIx64													"llx"
//#define PRIX64													"llX"
//#define PRIzu													"lu"
//#define PRIzd													"ld"
//#define PRTime													PRI64
//#else
//typedef int64_t													int64;
//typedef int32_t													int32;
//typedef int16_t													int16;
//typedef int8_t													int8;
//typedef uint64_t												uint64;
//typedef uint32_t												uint32;
//typedef uint16_t												uint16;
//typedef uint8_t													uint8;
//typedef uint16_t												WORD;
//typedef uint32_t												DWORD;
//#endif
//

//end define common data type. 

#include "cocos2d.h"

#include "KBEClientCoreMacros.h"
#include "net/base_sock.h"
#include "net/message.h"
#include "net/net_client.h"

#include "util/basic_types.h"
#include "util/byteorder.h"
#include "util/byte_buffer.h"
#include "util/tools.h"
#include "util/ReadUtil.h"
#include "util/MapUtil.h"
#include "util/TimeUtil.h"

//#include "common/MacrosDefine.h"
//#include "common/SearchPath.h"
//#include "common/WeightedDirectedGraph.h"
//#include "common/PlistManager.h"
//#include "common/AnimateManager.h"
//#include "common/ParticleManager.h"
//#include "common/InternationManager.h"
//#include "common/Effect.h"
//#include "common/EffectNode.h"
//#include "common/EffectManager.h"
//#include "common/BGMap.h"
//#include "common/TextureManager.h"
//#include "common/PerformanceMgr.h"
//
//#include "gui/WXUIDefine.h"
//#include "gui/WXSimpleButton.h"
//#include "gui/WXSimplePanel.h"
//#include "gui/WXPanel.h"
//#include "gui/WXRichLabelEx.h"
//#include "gui/WXGrid.h"
//#include "gui/WXGridPanel.h"
//#include "gui/WXGridPanelEx.h"
//#include "gui/WXColorRect.h"
//#include "gui/WXCheckButton.h"
//#include "gui/WXCheckButtonPanel.h"
//#include "gui/WXLinkLabel.h"
//#include "gui/WXMenu.h"
//#include "gui/WXRadioButton.h"
//#include "gui/WXRadioButtonPanel.h"
//#include "gui/WXMessageBox.h"
//#include "gui/WXTableView.h"
//#include "gui/WXTableViewConstantCellSize.h"



USING_NS_GC;