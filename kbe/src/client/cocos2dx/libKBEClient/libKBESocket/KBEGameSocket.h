
#pragma once

#include "GameNetClient.h"

class KBEGameSocket: public cocos2d::CCObject
{
private:
	KBEGameSocket();
	KBEGameSocket(const KBEGameSocket&);
	KBEGameSocket& operator=(const KBEGameSocket&);
	~KBEGameSocket();
public:
	static KBEGameSocket& getInstance()
	{
		static KBEGameSocket instance;
		return instance;
	}
	bool connectionServer(const char* ipStr, uint32 port=8001);
	void sendMessage(Message& msg);
	void sendData(char* data,int size);  
	//releasesockte
	void releaseSocket();
	//´´½¨socket
	void createSocket();
private:
	GameNetClient* gameSocket;
	std::string ip;
	uint32 port;
};

