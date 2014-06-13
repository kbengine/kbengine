
#include "SocketClient.h"
#include "MessageHandler.h"
//#include "DataEnvironment.h"
//#include "GameMessageFactory.h"
//#include "ResourceDataManager.h"
//#include "GameNetworkInterface.h"
#include "MSAutoReleasePool.h"
#include "message.h"
#include <errno.h>
#include <signal.h>
//#include "CData.h"

//CCDictionary* SocketClient::m_dictionary= cocos2d::CCDictionary::create();

SocketClient::SocketClient(String host, int port, byte clientid,
                           byte serverid,MessageHandler* netImpl):
m_iState(SocketClient_WAIT_CONNECT),
m_cbRecvBuf(1024*60),m_cbSendBuf(1024*60),
m_pHandler(netImpl)
{   
	m_isvalidSeq = false;
	memset(m_sabcde, 0, 6*8);
	
    pthread_mutex_init (&m_sendqueue_mutex,NULL);
	pthread_mutex_init(&m_thread_cond_mutex,NULL);
	pthread_cond_init(&m_threadCond, NULL);
	
	m_hSocket = -1;
	
	this->m_host = host;
	this->m_iport = port;
	this->m_clientId = clientid;
	this->m_serverId = serverid;
	
	m_bThreadRecvCreated = false;
	m_bThreadSendCreated = false;
    
    
    //    m_dictionary->retain();  //字典 存放 数据
    //    if(m_dictionary)
    //    {
    //            m_dictionary->removeAllObjects();
    //    }
}
void SocketClient::start(){
	if(!m_bThreadSendCreated){
		pthread_create( &pthread_t_send, NULL,ThreadSendMessage, this);
		m_bThreadSendCreated = true;
	}
}

bool SocketClient::isWaitConnect(){
	return m_iState == SocketClient_WAIT_CONNECT;
}
SocketClient::~SocketClient()
{
	m_iState = SocketClient_DESTROY;
	if( m_hSocket!=-1)
		close(m_hSocket);
	
	pthread_mutex_destroy(&m_sendqueue_mutex);
	pthread_mutex_destroy(&m_thread_cond_mutex);
	pthread_cond_destroy(&m_threadCond);
    
	
	while (!m_receivedMessageQueue.empty()) {
		Message* m = m_receivedMessageQueue.front();
		m_receivedMessageQueue.pop();
		SAFE_DELETE_ELEMENT(m);
	}
	
	while (!m_sendMessageQueue.empty()) {
		Message* m = m_sendMessageQueue.front();
		m_sendMessageQueue.pop();
		SAFE_DELETE_ELEMENT(m);
	}
	
    
	//if( DEBUG) printf("SocketClient::~SocketClient(),%p,%s:%d\n",this,m_host.c_str(),m_iport);
    
}

Message* constructErrorMessage(int type,int errCode,string error){
	
	Message* msg = new Message();
    //	msg->type = 0;
    //	msg->type_selfdefine = type;//TYPE_SELF_DEINE_MESSAGE_CONNECT_FAIL;
    //	ByteBuffer* buf = new ByteBuffer(1024);
    //	buf->putInt(errCode);
    //	buf->putUTF(error);
    //
    //	msg->data = buf->toByteArray();
    //	delete buf;
	
	return msg;
}

int SocketClient::bytesToInt(byte* bytes)
{
    
    int addr = bytes[3] & 0xFF;
    
    addr |= ((bytes[2] << 8) & 0xFF00);
    
    addr |= ((bytes[1] << 16) & 0xFF0000);
    
    addr |= ((bytes[0] << 24) & 0xFF000000);
    
    return addr;
    
}

byte* SocketClient::intToByte(int i)
{
    
    byte* abyte0 = new byte[4];
    
    abyte0[3] = (byte) (0xff & i);
    
    abyte0[2] = (byte) ((0xff00 & i) >> 8);
    
    abyte0[1] = (byte) ((0xff0000 & i) >> 16);
    
    abyte0[0] = (byte) ((0xff000000 & i) >> 24);
    
    return abyte0;
    
}

Message* SocketClient::constructMessage(const char* data,int commandId)
{
    Message* msg = new Message();
    
    msg->HEAD0=78;
    msg->HEAD1=37;
    msg->HEAD2=38;
    msg->HEAD3=48;
    msg->ProtoVersion=9;
    
    int a=0;
    msg->serverVersion[3]=(byte)(0xff&a);;
    msg->serverVersion[2]=(byte)((0xff00&a)>>8);
    msg->serverVersion[1]=(byte)((0xff0000&a)>>16);
    msg->serverVersion[0]=(byte)((0xff000000&a)>>24);
    
    int b=strlen(data)+4;
    
    msg->length[3]=(byte)(0xff&b);;
    msg->length[2]=(byte)((0xff00&b)>>8);
    msg->length[1]=(byte)((0xff0000&b)>>16);
    msg->length[0]=(byte)((0xff000000&b)>>24);
    
    int c=commandId;
    msg->commandId[3]=(byte)(0xff&c);;
    msg->commandId[2]=(byte)((0xff00&c)>>8);
    msg->commandId[1]=(byte)((0xff0000&c)>>16);
    msg->commandId[0]=(byte)((0xff000000&c)>>24);
    
    //    str.append(msg->HEAD0);
    printf("%d" ,msg->datalength());
    msg->data = new char[msg->datalength()];
    memcpy(msg->data+0,&msg->HEAD0,1);
    memcpy(msg->data+1,&msg->HEAD1,1);
    memcpy(msg->data+2,&msg->HEAD2,1);
    memcpy(msg->data+3,&msg->HEAD3,1);
    memcpy(msg->data+4,&msg->ProtoVersion,1);
    memcpy(msg->data+5,&msg->serverVersion,4);
    memcpy(msg->data+9,&msg->length,4);
    memcpy(msg->data+13,&msg->commandId,4);
    memcpy(msg->data+17,data,strlen(data));
    //memcpy(msg->data+position,bytes+offset,len);
    //msg->data = data;
	return msg;
}




void SocketClient:: stop(boolean b){
	m_iState = SocketClient_DESTROY;
	
	{
		MyLock lock(&m_thread_cond_mutex);
		pthread_cond_signal(&m_threadCond);
	}
	if(m_bThreadRecvCreated)
		pthread_join(pthread_t_receive, NULL);
	pthread_join(pthread_t_send, NULL);
}



bool SocketClient::connectServer()
{
    
	if( m_host.length() < 1 || m_iport == 0) return false;
    //	if( DEBUG){
    //		printf("[SocketClient::Connect()] [ host:%s,port:%d ] \n",m_host.c_str(),m_iport);
    //	}
	int dwServerIP = inet_addr(m_host.c_str());
	unsigned short wPort = m_iport;
    
	if( m_hSocket != -1){
		close(m_hSocket);
	}
    
	m_hSocket = socket(AF_INET,SOCK_STREAM,0);
	if (m_hSocket == -1)
	{
		return false;
	}
    
    //    int opt = 1;
    //    setsockopt(m_hSocket, SOL_SOCKET,SO_REUSEADDR, &opt,sizeof(&opt) );
    //	int i;
    //	int portClient[] = {13001,23002};
    //    int portlen = 2;
    //	for( i=0;i< portlen;i++){
    //		sockaddr_in SocketAddrClient;
    //		memset(&SocketAddrClient,0,sizeof(SocketAddrClient));
    //		SocketAddrClient.sin_family=AF_INET;
    //		SocketAddrClient.sin_port=htons(portClient[i]);
    //        //		SocketAddrClient.sin_addr.s_addr=htonl(INADDR_ANY);
    //        //		bzero(&(SocketAddrClient.sin_zero), 8);
    //		int bt = bind(m_hSocket,(sockaddr*)&SocketAddrClient,sizeof(SocketAddrClient));
    //
    //		printf("socket bind port %d , result=%d\n",portClient[i],bt);
    //		if( 0 == bt){
    //			break;
    //		}
    //	}
    //	if( i>=portlen){
    //		printf("socket bind fail\n");
    //		return false;
    //	}
    
	
	sockaddr_in SocketAddr;
	memset(&SocketAddr,0,sizeof(SocketAddr));
    
	SocketAddr.sin_family=AF_INET;
	SocketAddr.sin_port=htons(wPort);
    
	SocketAddr.sin_addr.s_addr=dwServerIP;
    
    memset(&(SocketAddr.sin_zero),0,sizeof(SocketAddr.sin_zero));
    
	int iErrorCode=0;
    
    
	iErrorCode= connect(m_hSocket,(sockaddr*)&SocketAddr,sizeof(SocketAddr));
	if (iErrorCode==-1)
	{
		printf("socket connect error:%d\n",errno);
		return false;
	}
    
	
	if( !m_bThreadRecvCreated ){
        
		if(pthread_create( &pthread_t_receive, NULL,ThreadReceiveMessage, this)!=0)
			return false;
		m_bThreadRecvCreated = true;
	}
    
	m_iState = SocketClient_OK;
	
	
	
    //if( DEBUG)	printf("socket connected success[ %s,%d],%p ,%d,%d \n",m_host.c_str(),m_iport, this,m_iState,SocketClient_OK);
	
	return true;
}

void SocketClient::sendMessage_(Message* msg,bool b)
{
	if(m_iState == SocketClient_DESTROY){
		delete msg;
		return;
	}
    
	{
		MyLock lock(&m_sendqueue_mutex);
		m_sendMessageQueue.push(msg);
	}
	if( m_iState == SocketClient_OK)
	{
		MyLock lock(&m_thread_cond_mutex);
		pthread_cond_signal(&m_threadCond);
	}
	
}

void* SocketClient::ThreadSendMessage(void *p){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	MSAutoReleasePool POOL;
#endif
	SocketClient* This = static_cast<SocketClient*>(p) ;
	
	while(This->m_iState == SocketClient_WAIT_CONNECT && !This->connectServer()){
		
		if( This->m_iport2.size()> 0){
			This->m_host = This->m_host2[0];
			This->m_iport = This->m_iport2[0];
			This->m_serverId = This->m_serverId2[0];
			This->m_clientId = This->m_clientId2[0];
			
			This->m_host2.erase(This->m_host2.begin());
			This->m_iport2.erase(This->m_iport2.begin());
			This->m_serverId2.erase(This->m_serverId2.begin());
			This->m_clientId2.erase(This->m_clientId2.begin());
		}
		else{
			This->m_iState = SocketClient_DESTROY;
			string error("连网失败,请检查网络设置");
            //		char tmp[64];
            //		sprintf(tmp, "%d",This->m_iport);
            //		error.append(This->m_host).append(":").append(tmp);
			{
				MyLock lock(&This->m_sendqueue_mutex);
                
				This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CONNECT_FAIL,errno,error));
			}
            //			printf("[SocketClient::start()][connect failed][error:%s] \n",error.c_str());
			return ((void *)0);
		}
	}
	
	ByteBuffer& sendBuff = This->m_cbSendBuf;
	int socket = This->m_hSocket;
	
	while (This->m_iState != SocketClient_DESTROY) {
		if( This->m_iState == SocketClient_OK){
            //发送缓冲器有遗留的数据要发送
            if(sendBuff.getPosition() > 0){
                sendBuff.flip();
                int ret = send(socket,(char *)sendBuff.getBuffer(),sendBuff.getLimit(),0);
                if(ret == -1){
                    This->m_iState = SocketClient_DESTROY ;
                    
                    string err("发送数据，网络异常！");
                    //				char* errStr = strerror(errno);
                    //				if( errStr!=NULL ) err.append(errStr);
                    
                    MyLock lock(&This->m_sendqueue_mutex);
                    This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CANNOT_SEND_MESSAGE,errno,err));
                    return ((void *)0);
                }else{
                    //printf("SocketClient::ThreadSendMessage(), send message to server, size = %d\n",ret);
                }
                sendBuff.setPosition(sendBuff.getPosition()+ret);
                sendBuff.compact();
            }
            
            Message* msg = NULL;
            while( This->m_iState != SocketClient_DESTROY && This->m_sendMessageQueue.size()> 0){
                {
                    MyLock lock(&This->m_sendqueue_mutex);
                    msg = This->m_sendMessageQueue.front();
                    This->m_sendMessageQueue.pop();
                }
                
                printf(" sendData length: %d  %ld" ,  msg->datalength(), sizeof(char));
                if(msg->datalength() + sendBuff.getPosition() > sendBuff.getLimit()){
                    This->m_iState = SocketClient_DESTROY;
                    printf("send buffer is full, send thread stop!");
                    MyLock lock(&This->m_sendqueue_mutex);
                    This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CANNOT_SEND_MESSAGE,0,"发送缓冲器已满，您的网络环境好像出现了问题！"));
                    return ((void *)0);
                }
                //			printf("send message %0x \n",msg->type);
                sendBuff.put(msg->data,0,msg->datalength());
                //            sendBuff.put(&msg, 0, msg->datalength());
                sendBuff.flip();
                int ret = send(socket,(char *)sendBuff.getBuffer(),sendBuff.getLimit(),0);
                if(ret == -1){
                    This->m_iState = SocketClient_DESTROY;
                    string err("发送数据，网络异常！");
                    MyLock lock(&This->m_sendqueue_mutex);
                    This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CANNOT_SEND_MESSAGE,errno,err));
                    return ((void *)0);
                }
                //			else{
                //				printf("SocketClient::ThreadSendMessage(), send message to server, size = %d\n",ret);
                //			}
                sendBuff.setPosition(sendBuff.getPosition()+ret);
                sendBuff.compact();
                
                delete msg;
            }
		}
		
		if(This->m_iState != SocketClient_DESTROY && This->m_sendMessageQueue.size() == 0){
			//sleep
			struct timeval tv;
			struct timespec ts;
			gettimeofday(&tv, NULL);
			ts.tv_sec = tv.tv_sec + 5;
			ts.tv_nsec = 0;
			
			MyLock lock(&(This->m_thread_cond_mutex));
			if(This->m_iState != SocketClient_DESTROY && This->m_sendMessageQueue.size() == 0){
				pthread_cond_timedwait(&(This->m_threadCond),&(This->m_thread_cond_mutex),&ts);
			}
			
		}
		
	}
	//if(DEBUG) printf("SocketClient::ThreadSendMessage(), send thread stop!\n");
	return (void*)0;
}
bool g_bcheckReceivedMessage = true;
void* SocketClient::ThreadReceiveMessage(void *p)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	MSAutoReleasePool POOL;
#endif
	fd_set fdRead;
	
	struct timeval	aTime;
	aTime.tv_sec = 1;
	aTime.tv_usec = 0;
    
	//最大多少秒，连接上收不到数据就提示用户，重新登录
	int maxIdleTimeInSeconds = 60*3;
	
	//最大多少秒，连接上收不到数据就提示用户，选择重连
	int hint2TimeInSeconds = 60;
	
	
	//多长时间没有收到任何数据，提示用户
	int hintTimeInSeconds = 30;
	
	struct timeval lastHintUserTime;
	struct timeval lastReceiveDataTime;
	struct timeval now;
	
	gettimeofday(&lastReceiveDataTime, NULL);
	lastHintUserTime = lastReceiveDataTime;
	
	SocketClient* This = static_cast<SocketClient*>(p) ;
	
	ByteBuffer* recvBuff = &This->m_cbRecvBuf;
    
	while (This->m_iState != SocketClient_DESTROY)
	{
        
//        CCLog("hahahahahahahhahahahahahahahha");
		if( This->m_iState != SocketClient_OK){
			usleep(1000);
			continue;
		}
		FD_ZERO(&fdRead);
        
		FD_SET(This->m_hSocket,&fdRead);
        
		aTime.tv_sec = 1;
		aTime.tv_usec = 0;
		
//		int ret = select(This->m_hSocket+1,&fdRead,NULL,NULL,NULL);
        int ret = select(This->m_hSocket+1,&fdRead,NULL,NULL,&aTime);
        
		if (ret == -1 )
		{
			if(errno == EINTR){
				printf("======   收到中断信号，什么都不处理＝＝＝＝＝＝＝＝＝");
			}else{
				This->m_iState = SocketClient_DESTROY;
                //				if(DEBUG)printf("select error, receive thread stop! errno=%d, address=%p\n",errno,This);
				MyLock lock(&This->m_sendqueue_mutex);
				This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CONNECT_TERMINATE,errno,"连接异常中断"));
				return ((void *)0);
			}
		}
		else if(ret==0)
		{
            //			printf("selector timeout . continue select.... \n");
			gettimeofday(&now, NULL);
			if( g_bcheckReceivedMessage ){
                if(now.tv_sec - lastReceiveDataTime.tv_sec > maxIdleTimeInSeconds && now.tv_sec - lastHintUserTime.tv_sec > hintTimeInSeconds){
                    lastHintUserTime = now;
                    
                    MyLock lock(&This->m_sendqueue_mutex);
                    
                    while( This->m_receivedMessageQueue.size()>0){
                        Message* msg = This->m_receivedMessageQueue.front();
                        This->m_receivedMessageQueue.pop();
                        CCLog("删除消息");
                        delete msg;
                    }
                    This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_RECONNECT_FORCE,0,"您的网络已经出问题了！"));
                    
                }else if(now.tv_sec - lastReceiveDataTime.tv_sec > hint2TimeInSeconds && now.tv_sec - lastHintUserTime.tv_sec > hintTimeInSeconds){
                    lastHintUserTime = now;
                    MyLock lock(&This->m_sendqueue_mutex);
                    This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_RECONNECT_HINT,0,"您的网络好像出问题了！"));
                }else if(now.tv_sec - lastReceiveDataTime.tv_sec > hintTimeInSeconds && now.tv_sec - lastHintUserTime.tv_sec > hintTimeInSeconds){
                    lastHintUserTime = now;
                    MyLock lock(&This->m_sendqueue_mutex);
                    This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_IDLE_TIMEOUT,0,"您的网络好像出问题了！"));
                }
            }else{
                lastHintUserTime = now;
                lastReceiveDataTime= now;
            }
		}
		else if (ret > 0)
		{
			if (FD_ISSET(This->m_hSocket,&fdRead))
			{
				int iRetCode = 0;
                printf(" recv data %d \n", recvBuff->remaining());
				if(recvBuff->remaining() > 0){
					iRetCode = recv(This->m_hSocket,recvBuff->getBuffer()+recvBuff->getPosition(),
                                    recvBuff->remaining(),0);
				}
                
                printf(" recv data later  %d   %d \n", recvBuff->remaining(), iRetCode);
				if (iRetCode == -1)
				{
					This->m_iState = SocketClient_DESTROY;
					MyLock lock(&This->m_sendqueue_mutex);
					
					while( This->m_receivedMessageQueue.size()>0){
						Message* msg = This->m_receivedMessageQueue.front();
						This->m_receivedMessageQueue.pop();
                        CCLog("删除消息");
						delete msg;
					}
					
					string tmp("网络连接中断！");
                    //					char* stre = strerror(errno);
                    //					if( stre!=NULL){
                    //						tmp.append(stre);
                    //					}
					This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CONNECT_TERMINATE,errno,tmp));
					return ((void *)0);
				}else if(iRetCode == 0 && recvBuff->remaining() > 0){
					This->m_iState = SocketClient_DESTROY;
                    //					if(DEBUG)printf("server closed connection, receive thread stop %p!\n",This);
					MyLock lock(&This->m_sendqueue_mutex);
					while( This->m_receivedMessageQueue.size()>0){
						Message* msg = This->m_receivedMessageQueue.front();
						This->m_receivedMessageQueue.pop();
                        CCLog("删除消息");
						delete msg;
					}
					This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_SERVER_CLOSE_CONNECTION,errno,"服务器主动关闭连接!"));
					
					return ((void *)0);
				}
				else
				{
					gettimeofday(&lastReceiveDataTime, NULL);
					
					recvBuff->setPosition(recvBuff->getPosition()+ iRetCode);
					recvBuff->flip();
					int tmpOffset = 17;
					while(recvBuff->remaining() > tmpOffset){
						int pos = recvBuff->position;
						int length= recvBuff->getLength(9);
						
						if(recvBuff->remaining()+tmpOffset >= length){

							Message* message = new Message();
  
                            message->HEAD0 = recvBuff->getByte();
                            message->HEAD1 = recvBuff->getByte();
                            message->HEAD2 = recvBuff->getByte();
                            message->HEAD3 = recvBuff->getByte();
                            message->ProtoVersion = recvBuff->getByte();
                            recvBuff->getAsBytes(message->serverVersion);
                            recvBuff->getAsBytes(message->length);
                            recvBuff->getAsBytes(message->commandId);
                            
							printf("message length: %d commandId: %d \n", bytesToInt(message->length),bytesToInt(message->commandId));
                            
							char* tmp = new char[length-3];
							recvBuff->get(tmp,0,length-4);
                            tmp[length-4] = '\0';
                            message->data = tmp;
					
                                
                                MyLock lock(&This->m_sendqueue_mutex);

                                This->m_receivedMessageQueue.push(message);
                                CCLog("接收队列长度:::: %d",sizeof(This->m_receivedMessageQueue));
                                
                                CCLog("%d-----------------",This->m_receivedMessageQueue.size());
                                
                                CCLog("%d",bytesToInt(message->commandId));
                                if(bytesToInt(message->commandId)==218){
                                    ;//CData::getCData()->m_newlevel_dic->setObject(message, 218);
                                }
                                else{
                                    ;//CData::getCData()->m_dictionary->setObject(message,bytesToInt(message->commandId));
                                    
                                }
							
                            
						}else if(length>recvBuff->getCapacity()){
							This->m_iState = SocketClient_DESTROY;
                          
							MyLock lock(&This->m_sendqueue_mutex);
							
							while( This->m_receivedMessageQueue.size()>0){
								Message* msg = This->m_receivedMessageQueue.front();
								This->m_receivedMessageQueue.pop();
                                CCLog("删除消息");
								delete msg;
							}
							
							This->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CONNECT_TERMINATE,0,"数据包太大，连接中断！"));
							return ((void *)0);
						}else {
							//printf("----------------------------\n");
							recvBuff->position = pos;
							break;
						}
					}
					//
					recvBuff->compact();
				}
				
			}//end read
		}
        
        
	}
	//if(DEBUG) printf("SocketClient::ThreadSendMessage(), receive thread stop!\n");
    
	return (void*)0;
}
//获取服务器包
Message* SocketClient::pickReceivedMessage(){
	Message* msg = NULL;
	MyLock lock(&m_sendqueue_mutex);
	if( m_receivedMessageQueue.size()>0){
		
		msg = m_receivedMessageQueue.front();
        
	}
	
	return msg;
}
Message* SocketClient::popReceivedMessage(){
	Message* msg = NULL;
	MyLock lock(&m_sendqueue_mutex);
	if( m_receivedMessageQueue.size()>0){
		
		msg = m_receivedMessageQueue.front();
		m_receivedMessageQueue.pop();
        
	}
    
	
	return msg;
}

void SocketClient::pushReceivedMessage(Message* msg){
	MyLock lock(&m_sendqueue_mutex);
	m_receivedMessageQueue.push(msg);
}

long long SocketClient::getSeq(){
	if( m_isvalidSeq ){
		long long a = m_sabcde[1];
		long long b = m_sabcde[2];
		long long c = m_sabcde[3];
		long long d = m_sabcde[4];
		m_sabcde[0] = (long long)(a*2+b+c*3+d);
		m_sabcde[1] = a^b+b|c+3+d;
		m_sabcde[2] = b-a+d*123;
		m_sabcde[3] = (c%123456)+a*b+(long long)sqrt((double)abs(d));
		m_sabcde[4] = (long long)(a*1.233f+b*0.45456f+c*d+9);
	}
	return m_sabcde[0];
}






void SocketClient::swhlie(int commandId)
{
    
    
}






















///////////////////server////////////
SocketServer* g_socketServer = new SocketServer();

SocketServer::SocketServer(){
    isStartedServer = false;
    isExistOtherServer = false;
    server_socket = 0;
    isStartServerBindFail = false;
}

SocketServer::~SocketServer(){
    if( server_socket!=0){
        close(server_socket);
        server_socket=0;
    }
}


void* ThreadListen(void* p){
    SocketServer* serv = (SocketServer*)p;
    char tmp[256]="";
    while (serv->isStartedServer) //服务器端要一直运行
    {
        //定义客户端的socket地址结构client_addr
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        //接受一个到server_socket代表的socket的一个连接
        //如果没有连接请求,就等待到有连接请求--这是accept函数的特性
        //accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
        //new_server_socket代表了服务器和客户端之间的一个通信通道
        //accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
        int new_server_socket = accept(serv->server_socket,(struct sockaddr*)&client_addr,&length);
        if ( new_server_socket < 0)
        {
            //if( DEBUG)            printf("[SocketServer] Server Accept Failed!\n");
            continue;
        }
        memset(tmp, 0, 256);
        int rt = recv(new_server_socket, tmp, 256,0);
        //关闭与客户端的连接
        close(new_server_socket);
        if( rt>0 && strcmp(tmp , "kill wxs" ) == 0){
            serv->killed();
            //if( DEBUG)        printf("[SocketServer] recv message [%s] from new client!\n",tmp);
            break;
        }
    }
    //关闭监听用的socket
    if( serv->server_socket !=0 ){
        close(serv->server_socket);
        serv->server_socket = 0;
    }
    serv->isStartedServer = false;
    serv->isStartServerBindFail = false;
    return 0;
}


void SocketServer::killed(){
    isStartedServer = false;
    isStartServerBindFail = false;
    //    if( DataEnvironment::netImpl->getGateway()!=NULL ){
    //    DataEnvironment::netImpl->getGateway()->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CLIENT_KILL_MESSAGE,errno,""));
    //    }
    //    if( DataEnvironment::netImpl->getServer()!=NULL ){
    //        DataEnvironment::netImpl->getServer()->m_receivedMessageQueue.push(constructErrorMessage(TYPE_SELF_DEINE_MESSAGE_CLIENT_KILL_MESSAGE,errno,""));
    //    }
}
bool SocketServer::startServer(){
    if( server_socket!=0){
        close(server_socket);
        server_socket= 0;
    }
    if( !isStartedServer){
        usleep(1000000);
        isStartServerBindFail = false;
        struct sockaddr_in server_addr;
        int HELLO_WORLD_SERVER_PORT = 43067;
        memset(&server_addr,0,sizeof(server_addr)); //把一段内存区的内容全部设置为0
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htons(INADDR_ANY);
        server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
        //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
        server_socket = socket(AF_INET,SOCK_STREAM,0);
        if( server_socket < 0)
        {
            //if( DEBUG)        printf("[SocketServer] Create Socket for SocketServer Failed!\n");
            return false;
        }
        int opt =1;
        setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        
        //把socket和socket地址结构联系起来
        
        if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
        {
            isStartServerBindFail = true;
            //if( DEBUG)        printf("[SocketServer] Server Bind Port : %d Failed!\n", HELLO_WORLD_SERVER_PORT);
            return false;
        }
        //server_socket用于监听
        if ( listen(server_socket, 5) )
        {
            //  if( DEBUG)        printf("[SocketServer] Server Listen Failed!\n");
            return false;
        }
        isStartedServer = true;
        // if( DEBUG)    printf("[SocketServer] Server started!\n");
        pthread_t t;
        pthread_create( &t, NULL,ThreadListen, this);
    }
    return true;
}

void SocketServer::killOtherServer(){
    isExistOtherServer = false;
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        return;
    }
    
    int HELLO_WORLD_SERVER_PORT = 43067;
    
    sockaddr_in SocketAddr;
	memset(&SocketAddr,0,sizeof(SocketAddr));
	SocketAddr.sin_family=AF_INET;
	SocketAddr.sin_port=htons(HELLO_WORLD_SERVER_PORT);
	SocketAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	memset(&(SocketAddr.sin_zero),0, 8);
    
	int iErrorCode=0;
    
    
	iErrorCode= connect(client_socket,(sockaddr*)&SocketAddr,sizeof(SocketAddr));
	if (iErrorCode==-1)
	{
        // if( DEBUG) printf("[CheckServer] socket server not exist error:%d\n",errno);
        close( client_socket);
		return;
	}
    const char *tmp = "kill wxs";
    write(client_socket, tmp, strlen(tmp));
    // if( DEBUG)   printf("[CheckServer] socket server : kill  server\n");
    
    
    char str[256];
    memset(str, 0, 256);
    int rt = recv(client_socket, str, 256, 0);
    
    close( client_socket);
    
    if( rt>0 ){
        isExistOtherServer = true;
        // if( DEBUG) printf("[SocketServer] recv message [%s] from server!\n",str);
    }
}
