/*
 *  MessageHandler.h
 *  test
 *
 *  Created by admin on 11-8-18.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _H_MessageHandler_H
#define _H_MessageHandler_H
class SocketClient;
class Message;
class MessageHandler {	
public:	
	virtual SocketClient* getGateway() = 0;
	virtual SocketClient* getServer() = 0;
	/**
	 * 从网络上收到一个消息，此消息可能是请求消息，也可能是响应消息
	 * 需要返回一个响应。 如果想终止连接，可以选择抛出ConnectionException异常。其他异常不会终止连接。
	 * 
	 * @param gc
	 * @param request
	 * @return
	 * @throws ConnectionException
	 */
	virtual void receiveMessage(SocketClient* gc,Message* message)=0;
	
	virtual void clearReceivedMessage()=0;
	/**
	 * 长时间没有消息发送给服务器，调用此方法。 此方法可以返回一个消息，此返回的消息将放送给服务器。
	 * 
	 * @return
	 */
	virtual Message* idleTimeout()=0;
	
	/**
	 * 收到HTTP应答
	 * @param type
	 * @param data
	 * @param header
	 */
	virtual void httpReceived(int type,ByteBuffer* data,String header)=0;
	
	static const int ILLEGAL_ARGUMENT = 0;
	static const int CONNECTION_NOT_FOUND = 1;
	static const int SECURITY_PERMISSION = 2;
	static const int OTHER_IO_ERROR = 3;       
};


#endif
