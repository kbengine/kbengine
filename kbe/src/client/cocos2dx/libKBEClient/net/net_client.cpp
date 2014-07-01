#include "net_client.h"
#include "cocos2d.h"
//#include "util/byteorder.h"
#include <stdio.h>
#include "KMemoryStream.h"
#include "KMessage.h"
#include "KBEApplication.h"
#include "../client/KAccount.h"


USING_NS_CC;

NS_GC_BEGIN

#define RECV_LEN 5840//4096

MemoryStream stream;

NetClient::NetClient()
{
	if (!InitSocketSystem())
	{
		printf("Failed to retrive socket version.\n");
		return;
	}
	sock = new BaseSock();
	sock->Create(false);
	isConnect = false;
	messageLengthLenC2S = 2;
	messageOpcodeLenC2S = 2;
	messageLengthLenS2C = 2;
	messageOpcodeLenS2C = 2;
	CCDirector::sharedDirector()->getScheduler()->scheduleSelector(schedule_selector(NetClient::tick), this, 0, false);
}

NetClient::~NetClient()
{
	if (isConnect)
		close();
	if (sock)
	{
		delete sock;
		sock = NULL;
	}
	CCDirector::sharedDirector()->getScheduler()->unscheduleSelector(schedule_selector(NetClient::tick), this);
}
void NetClient::setMessageLengthLenC2S(size_t len)
{
	messageLengthLenC2S = len;
}
void NetClient::setMessageOpcodeLenC2S(size_t len)
{
	messageOpcodeLenC2S = len;
}
void NetClient::setMessageLengthLenS2C(size_t len)
{
	messageLengthLenS2C = len;
}
void NetClient::setMessageOpcodeLenS2C(size_t len)
{
	messageOpcodeLenS2C = len;
}
bool NetClient::connect(const char* host, uint16 port)
{
	bool bRet = false;
	if (isConnect)
		close();
	if (sock)
	{
		if (!sock->Connect(host, port))
		{
			clearSocketSystem();
			bRet = false;
		}
		else
		{
			isConnect = true;
			bRet = true;
		}
	}
	else
		bRet = false;
	if (bRet)
	{
		CCDirector::sharedDirector()->getScheduler()->resumeTarget(this);
	}
	else
	{
		CCDirector::sharedDirector()->getScheduler()->pauseTarget(this);
	}
	return bRet;
}
void NetClient::close()
{
	if (sock)
	{
		sock->Close();
		clearSocketSystem();
		isConnect = false;
		CCDirector::sharedDirector()->getScheduler()->pauseTarget(this);

		delete sock;
		sock = NULL;
	}
}
void NetClient::tick(float dt)
{
	if (sock && isConnect)
	{
		char buf[RECV_LEN];
		memset(buf, 0, RECV_LEN);
		
		//select(sock, mode.??? 
		
		int len = sock->Recv(buf,RECV_LEN);
		if (len <= 0)
		{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
			/*
			if (len == SOCKET_ERROR )
			{
			}
			int err = WSAGetLastError(); 
			if (err==WSAECONNRESET ||err==WSAECONNABORTED)
			{
				close();
				noticeSocketDisconnect(len, err);
			}*/
			bool ret = false;  
			HANDLE closeEvent = WSACreateEvent();  
			WSAEventSelect(sock->m_sock, closeEvent, FD_CLOSE);  

			DWORD dwRet = WaitForSingleObject(closeEvent, 0);  

			if(dwRet == WSA_WAIT_EVENT_0)  
				ret = true;  
			else if(dwRet == WSA_WAIT_TIMEOUT)  
				ret = false;  
			WSACloseEvent(closeEvent);

			if (ret)//断开连接了
			{
				close();
				noticeSocketDisconnect(len, (int)dwRet);
			}
#else
			int sockErr = errno;
			if (len == -1 && EWOULDBLOCK == sockErr)//还是正常的
			{
			}
			else//断开连接了
			{
				close();
				noticeSocketDisconnect(len, sockErr);
			}
#endif
		}
		else
		{
			//
			CCLog("got data size=%d",len);
			//invoke memorystream to store resp packet.
			//msgReader.process(_datas, (MessageLength)successReceiveBytes);
			
			//msgid,2 length,2,body=(length)
			 
			//stream.data = (uint8*)buf;
			stream.append(buf,len);

			//until get whole packet trigger once call.
			//
			//stream.print_storage();
		
			//
			while(stream.opsize())
			{
				//stream.print_storage();
			/*	uint16 msgid;
				uint16 msglen;
				stream>>msgid;
				stream>>msglen;			
				uint8* msgbody = new uint8[msglen];*/
				//stream>>msgbody;
				//518 = Client_onImportClientMessages
				// 
				
				uint16 msgid;
				uint16 msglen;
					
				//check head full head?
				if ( stream.opsize() >= 4)
				{
					stream>>msgid;
					stream>>msglen;
				}
				else
				{
					//only when disconnect from loginapp call here.
					stream.clear(true);
					CCLog(" stream clear opsize = %d", stream.opsize() );
					break;				
				}

				//check body.
				if ( stream.opsize() < msglen-4)
				{
					size_t pos = stream.rpos();
					stream.rpos(pos-4);
					stream.clear(true);
					CCLog(" stream opsize =  %d", stream.opsize() );
					break;
				}
				CCLog("msgid=%d msglen=%d",msgid,msglen);
				//moved to clientapp?
				//at least a whole packet.

				if( msgid==518)
				{
					//stream.clear(true);
					//uint16 msgcount;
					//stream>> msgcount;
					KMessage::onImportClientMessages(stream);
					//stream.clear(true);
					continue;
					//how to check left data? 
				}
				if( msgid == 519)
				{
					//+		[56]	(519, 0x04c0af48 {id=519 msgname="Client_onImportClientEntityDef" msglen=-1 ...})

					KMessage::onImportClientEntityDef(stream);
					//stream.clear(true);
					continue;
				}
				if( msgid == 7){
					//
					KMessage::Client_onHelloCB(stream);
					//two lap packet.
					//stream.clear(true);
					//return;
					continue;
				}
				if (msgid == 8)
				{
					KMessage::Client_onVersionNotMatch(stream);
					//stream.clear(true);
					continue;
				}
				if ( msgid == 39) 
				{
					KMessage::Client_onImportMercuryErrorsDescr(stream);
					//stream.clear(true);
					continue;
				}
				if ( msgid == 502) //503 login failed.
				{
					//loginSuccessfully
					KMessage::Client_onLoginSuccessfully(stream);
					//stream.clear(true);
					continue;
				}

				//
				if( msgid ==  KMessage::messages["Client_onCreatedProxies"]->id )
				{
					KMessage::Client_onCreatedProxies(stream);
					//stream.clear(true);
					continue;
				}

				if( msgid == 511)
				{
					//after create proxies, update propertys be called.
					KBEngineClient::ClientApp::getInstance().onUpdatePropertys(stream);
					continue;
				}

				if( KMessage::messages["Client_onRemoteMethodCall"] && msgid == KMessage::messages["Client_onRemoteMethodCall"]->id) 
				{
					//methodcall. 506
					KBEngineClient::ClientApp::getInstance().onRemoteMethodCall(stream);
					continue;
				}
				if( msgid == 505 ){
				
					uint16 failcode;
					stream>>failcode;
					KBEngineClient::ClientApp::getInstance().onLoginGatewayFailed(failcode);
					continue;
				}

				if ( msgid == 507){
					//enterworld
					KBEngineClient::ClientApp::getInstance().onEntityEnterWorld(stream);
					continue;
				}
				if( msgid == 508 )
				{
					//leaveworld
					continue;
				
				}
				if( msgid == 509)
				{	
					//enter space
					continue;
				
				}
				if( msgid == 510)
				{	
					//leave space		
					continue;
				}

				if( KMessage::messages["Client_onEntityDestroyed"] && msgid == KMessage::messages["Client_onEntityDestroyed"]->id)
				{
					//512
					//some entity destroyed.
					ENTITY_ID eid;
					stream >> eid;
					KBEngineClient::ClientApp::getInstance().onEntityDestroyed(eid);
					continue;
				}

				if( KMessage::messages["onCreateAvatarResult"] && msgid == KMessage::messages["onCreateAvatarResult"]->id)
				{
					KAccount* account = (KAccount*) KBEngineClient::ClientApp::getInstance().pPlayer();
					int ret;
					stream>>ret;
					account->onCreateAccountResult(ret);
					continue;
				}

				// no function handler. skip body and return
				stream.read_skip(msglen);
				CCLog(" msgid=%d be ignored! msglen=%d",msgid, msglen );
				//KMessage::onImportClientMessages(stream);
				//KBEngineClient::ClientApp::getInstance();
			};//end while 
			//
			//Todo:
			//  Message msg = Message[msgid]
			//  msg.handle(stream);
			//  
			//  Done.
			//return;
			
			//buffer.append(buf, len);
			////
			//while (buffer.remaing() >= messageLengthLenS2C)
			//{
			//	size_t resetLen = 0;
			//	switch (messageLengthLenS2C)
			//	{
			//	case 2:
			//		{
			//			uint16 messageLen;
			//			buffer >> messageLen;
			//			resetLen = endian_swap2<uint16> (messageLen);
			//			break;
			//		}
			//	case 4:
			//		{
			//			uint32 messageLen;
			//			buffer >> messageLen;
			//			resetLen = endian_swap4<uint32> (messageLen);
			//			break;
			//		}
			//	}

			//	if (buffer.remaing() >= resetLen)
			//	{
			//		Message message;

			//		switch (messageOpcodeLenS2C)
			//		{
			//		case 2:
			//			{
			//				uint16 opcode;
			//				buffer >> opcode;
			//				opcode = endian_swap2<uint16> (opcode);
			//				message.setOpcode(opcode);
			//				break;
			//			}
			//		case 4:
			//			{
			//				uint32 opcode;
			//				buffer >> opcode;
			//				opcode = endian_swap4<uint16> (opcode);
			//				message.setOpcode(opcode);
			//				break;
			//			}
			//		}
			//		uint32 msgLen = resetLen - messageOpcodeLenS2C;
			//		if (msgLen)
			//		{
			//			uint8* destBuf = new uint8[msgLen];
			//			buffer.read(destBuf, msgLen);
			//			message.getBuffer().append(destBuf, msgLen);
			//			delete destBuf;
			//		}
			//		handlMessage(message);
			//		CCLOG("recv: %d", message.getOpcode());
			//	}
			//	else
			//	{
			//		buffer.rpos(buffer.rpos() - messageLengthLenS2C);
			//		break;
			//	}
			//}
			//size_t remaing = buffer.remaing();
			//if(remaing)
			//{
			//	char* leftover = new char[remaing];
			//	memset(leftover, 0, remaing);
			//	buffer.read((uint8*) leftover, remaing);
			//	buffer.clear();
			//	buffer.append(leftover, remaing);
			//	delete leftover;
			//}
			//else
			//	buffer.clear();
		}
	}
}

//void NetClient::sendMessage(Message& message)
//{
//	if (sock && isConnect)
//	{
//		ByteBuffer tmp;
//
//		switch (messageLengthLenC2S)
//		{
//		case 2:
//			{
//				uint16 len = messageOpcodeLenC2S + message.getBuffer().size();
//				tmp << endian_swap2<uint16> (len);
//				break;
//			}
//		case 4:
//			{
//				uint32 len = messageOpcodeLenC2S + message.getBuffer().size();
//				tmp << endian_swap4<uint32> (len);
//				break;
//			}
//		}
//		switch (messageOpcodeLenC2S)
//		{
//		case 2:
//			{
//				uint16 opcode = message.getOpcode();
//				tmp << endian_swap2<uint16> (opcode);
//				break;
//			}
//		case 4:
//			{
//				uint32 opcode = message.getOpcode();
//				tmp << endian_swap4<uint32> (opcode);
//				break;
//			}
//		}
//		tmp.append(message.getBuffer());
//		uint32 remaing = tmp.size();
//		uint8* sendBuffer = new uint8[remaing];
//		tmp.read(sendBuffer, remaing);
//		sock->Send((char*) sendBuffer, remaing);
//		delete sendBuffer;
//		CCLOG("send: %d", message.getOpcode());
//	}
//}
void NetClient::noticeSocketDisconnect(int len, int sockErr)
{

}

void NetClient::sendData( char* data,int size )
{
	if (sock && isConnect)
	{
		sock->Send(data,size);
		CCLOG("send: %d",size);
	}
}

NS_GC_END