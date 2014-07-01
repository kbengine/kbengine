//
//  
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//
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

