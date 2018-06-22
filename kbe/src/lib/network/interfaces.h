// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NETWORK_INTERFACES_H
#define KBE_NETWORK_INTERFACES_H

namespace KBEngine { 
namespace Network
{
class Channel;
class MessageHandler;

/** 此类接口用于接收普通的Network输入消息
*/
class InputNotificationHandler
{
public:
	virtual ~InputNotificationHandler() {};
	virtual int handleInputNotification(int fd) = 0;
};

/** 此类接口用于接收普通的Network输出消息
*/
class OutputNotificationHandler
{
public:
	virtual ~OutputNotificationHandler() {};
	virtual int handleOutputNotification(int fd) = 0;
};

/** 此类接口用于接收一个网络通道超时消息
*/
class ChannelTimeOutHandler
{
public:
	virtual void onChannelTimeOut(Channel * pChannel) = 0;
};

/** 此类接口用于接收一个内部网络通道取消注册
*/
class ChannelDeregisterHandler
{
public:
	virtual void onChannelDeregister(Channel * pChannel) = 0;
};

/** 此类接口用于监听NetworkStats事件
*/
class NetworkStatsHandler
{
public:
	virtual void onSendMessage(const MessageHandler& msgHandler, int size) = 0;
	virtual void onRecvMessage(const MessageHandler& msgHandler, int size) = 0;
};


}
}

#endif // KBE_NETWORK_INTERFACES_H
