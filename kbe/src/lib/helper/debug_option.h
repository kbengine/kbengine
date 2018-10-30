// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_DEBUG_OPTION_H
#define KBE_DEBUG_OPTION_H
#include "common/common.h"

namespace KBEngine{

namespace Network
{

/** 
	这个开关设置数据包是否总是携带长度信息， 这样在某些前端进行耦合时提供一些便利
	 如果为false则一些固定长度的数据包不携带长度信息， 由对端自行解析
*/
extern bool g_packetAlwaysContainLength;

/**
是否需要将任何接收和发送的包以文本输出到log中提供调试
		g_trace_packet:
			0: 不输出
			1: 16进制输出
			2: 字符流输出
			3: 10进制输出
		use_logfile:
			是否独立一个log文件来记录包内容，文件名通常为
			appname_packetlogs.log
		g_trace_packet_disables:
			关闭某些包的输出
*/
extern uint8 g_trace_packet;
extern bool g_trace_encrypted_packet;
extern std::vector<std::string> g_trace_packet_disables;
extern bool g_trace_packet_use_logfile;

}

/**
	是否输出entity的创建， 脚本获取属性， 初始化属性等调试信息。
*/
extern bool g_debugEntity;

/**
	apps发布状态, 可在脚本中获取该值
		0 : debug
		1 : release
*/
extern int8 g_appPublish;

}

#endif // KBE_DEBUG_OPTION_H
