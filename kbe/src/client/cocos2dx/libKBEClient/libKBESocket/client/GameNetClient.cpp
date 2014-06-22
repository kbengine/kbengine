#include "GameNetClient.h"
#include "MessageHeader.h"
//#include "LoginController.h"
//#include "SceneController.h"
//#include "PlayerController.h"
//#include "PlayerData.h"
//#include "GameUIController.h"
//#include "TaskController.h"
//#include "DialogController.h"
//#include "ActivityManager.h"
//#include "PetManager.h"
//#include "GameSocket.h"
//#include "GameGobal.h"
//#include "GameUILayer.h"
//#include "SceneLayer.h"
//#include "InstanceManager.h"
//#include "FriendManager.h"
USING_NS_CC;

#define RECV_LEN 4096

GameNetClient::GameNetClient()
{
	heartBeatTime = 0;
}
void GameNetClient::handlMessage(Message& msg)
{
	uint16 messageID = msg.getOpcode();
	switch(messageID)
	{
	case msg_player_role_list_s2c:
		{
			//LoginController::getInstance().recRoleListS2C(msg);
			break;
		}
	default:
		break;
	}
}
void GameNetClient::tick(float dt)
{
	NetClient::tick(dt);
	return;//for test.
	if (sock && isConnect)
	{
		handleHeartBeat(dt);
	}
}
void GameNetClient::noticeSocketDisconnect(int len, int sockErr)
{
	CCLOG("socket disconnet!!!!!!!!!, len: %d, error: %d", len, sockErr);
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
	switch (sockErr)
	{
	case ETIMEDOUT:
		CCLOG("Connection timed out");
		break;
	case ECONNREFUSED:
		CCLOG("Connection refused");
		break;
	case EHOSTDOWN:
		CCLOG("Host is down");
		break;
	case EHOSTUNREACH:
		CCLOG("No route to host");
		break;
	}
#endif
	CCMessageBox("socket disconnet!","");
}
void GameNetClient::reConnectBtnCallback(void* result, void* bindData)
{

}
void GameNetClient::handleHeartBeat(float dt)
{
	heartBeatTime += dt;
	if (heartBeatTime>HEARTBEAT_INTERVAL)
	{
		Message msg;
		msg.setOpcode(msg_heartbeat_c2s);
		int beatTime = TimeUtil::getInstance().currentTimestamp1();
		msg << beatTime;
		sendMessage(msg);
		heartBeatTime = 0;
	}
}