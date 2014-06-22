#pragma once

#include "KBEClientcore.h"

#define HEARTBEAT_INTERVAL	20

class GameNetClient: public NetClient
{
public:
	GameNetClient();
	void handlMessage(Message& message);
	void noticeSocketDisconnect(int len, int sockErr);
	void tick(float dt);

private:
	void handleHeartBeat(float dt);
	void reConnectBtnCallback(void* result, void* bindData);//重新连接服务器



private:
	float heartBeatTime;
};