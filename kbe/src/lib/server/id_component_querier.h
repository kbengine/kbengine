// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_GET_COMPONENT_INFO_H
#define KBE_GET_COMPONENT_INFO_H

#include "common/common.h"
#include "network/common.h"
#include "network/bundle.h"
#include "network/endpoint.h"
#include "network/message_handler.h"

namespace KBEngine {

class IDComponentQuerier : public Network::Bundle
{
public:
	IDComponentQuerier();
	~IDComponentQuerier();

	void close();

	COMPONENT_ID query(COMPONENT_TYPE componentType, int32 uid);

	bool broadcast(uint16 port = 0);
	bool receive(Network::MessageArgs* recvArgs, sockaddr_in* psin = NULL, int32 timeout = 100000, bool showerr = true);

	bool good() const {
		return epListen_.good() && good_; 
	}

private:
	void send(COMPONENT_TYPE componentType, int32 uid);

private:
	Network::EndPoint epListen_, epBroadcast_;
	uint32 recvWindowSize_;
	int8 itry_;
	bool good_;
};

}

#endif
