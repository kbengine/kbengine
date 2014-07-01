#pragma once

#include "base_sock.h"
//#include "message.h"
#include "cocos2d.h"
#include "basic_types.h"

NS_GC_BEGIN

class NetClient: public cocos2d::CCObject
{
public:
	NetClient();
	virtual ~NetClient();
	virtual bool connect(const char* host, uint16 port);//连接socket
	virtual void close();//关闭socket
	virtual void tick(float dt);//更新
	//virtual void handlMessage(Message& message)=0;//接受消息
	//virtual void sendMessage(Message& message);//发送消息
	virtual void sendData(char* data,int size);
	virtual void noticeSocketDisconnect(int len, int sockErr);//通知socket断开连接
	void setMessageLengthLenC2S(size_t len);
	void setMessageOpcodeLenC2S(size_t len);
	void setMessageLengthLenS2C(size_t len);
	void setMessageOpcodeLenS2C(size_t len);
protected:
	BaseSock* sock;
	bool isConnect;
private:
	//ByteBuffer buffer;
	size_t messageLengthLenC2S;//客户端文件长度值大小
	size_t messageOpcodeLenC2S;//客户端消息头大小
	size_t messageLengthLenS2C;//客户端文件长度值大小
	size_t messageOpcodeLenS2C;//客户端消息头大小
};

NS_GC_END