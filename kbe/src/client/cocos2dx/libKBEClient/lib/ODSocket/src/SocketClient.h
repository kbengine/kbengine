/*
 *  SocketClient.h
 *  test
 *
 *  Created by qinxg on 7/28/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _CDATA_NETSOCKET_H_
#define _CDATA_NETSOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <queue>
#include <pthread.h>
#include <unistd.h>
#include "cocos2d.h"
#include "ByteBuffer.h"
const int	SocketClient_WAIT_CONNECT = 0;
const int	SocketClient_OK = 1;
const int	SocketClient_DESTROY = 2;


class Message;
class MessageHandler;

using namespace std;

class SocketClient{
public:
	int m_hSocket;

	char m_serverId;
	char m_clientId;
	string m_host;
	int m_iport;
//    static cocos2d::CCDictionary * m_dictionary;    //存放接收的数据
	vector<char> m_serverId2;
	vector<char> m_clientId2;
	vector<string> m_host2;
	vector<int> m_iport2;//链接服务器1失败后，如果服务器2存在将连服务器2
	
	//发送和接收缓冲区，发送缓冲区满的时候，会断开连接，并提示信号不好
	ByteBuffer m_cbRecvBuf;
	ByteBuffer m_cbSendBuf;
	
	//收到服务端消息
	queue<Message*> m_receivedMessageQueue;
	
	//需要发送到服务端的消息
	queue<Message*> m_sendMessageQueue;
	
	int m_iState;
	MessageHandler* m_pHandler;
	
	//接收线程
	bool m_bThreadRecvCreated;
	pthread_t pthread_t_receive;
	
	//发送线程
	bool m_bThreadSendCreated;
	pthread_t pthread_t_send;
	pthread_mutex_t m_thread_cond_mutex;//pthread_mutex_t 互斥锁
	pthread_cond_t m_threadCond;
	
    //聊天线程
    bool m_bThreadChatCreated;
    pthread_t pthread_t_chat;
    pthread_mutex_t m_thread_chat_mutex;
    //pthread_cond_t m_threadCond;
	//发送队列同步锁
	pthread_mutex_t m_sendqueue_mutex;
	
	bool m_isvalidSeq;
	long long m_sabcde[6];
	long long getSeq();
private:
	//连接服务器
	bool  connectServer();
	
	static void* ThreadReceiveMessage(void *p);
	static void* ThreadSendMessage(void *p);
	
public:
	SocketClient(String host, int port, byte clientId,
				 byte serverId,MessageHandler* netImpl);
	
	~SocketClient();
	void start();
	void stop(boolean b);
	
	bool isWaitConnect();
	//发送数据
	void sendMessage_(Message* msg,bool b);	
	
	Message* popReceivedMessage();
	Message* pickReceivedMessage();
	
	void pushReceivedMessage(Message* msg);
	
	MessageHandler* getHandler(){
        return  m_pHandler;
    }       
	
	void setHandler(MessageHandler* handler){
		this->m_pHandler = handler;
	}
	
    Message* constructMessage(const char* data,int commandId);
    static int bytesToInt(byte* data);
    static byte* intToByte(int i);
    void swhlie(int commandId);
    
};



class SocketServer{
public:
	bool isStartedServer;
    bool isExistOtherServer;
    bool isStartServerBindFail;
    
    int server_socket;    
   
    SocketServer( );
	~SocketServer();
    
	bool startServer();
    void killOtherServer();
    void killed();
};
#endif
