#include "KBEGameSocket.h"

KBEGameSocket::KBEGameSocket()
{
	gameSocket = NULL;
}
KBEGameSocket::~KBEGameSocket()
{
	releaseSocket();
}
bool KBEGameSocket::connectionServer(const char* ipStr, uint32 port)
{
	ip = ipStr;
	port = port;
	releaseSocket();
	createSocket();
	if(gameSocket->connect(ip.c_str(), port))
	{
		return true;
	}
	return false;
}
void KBEGameSocket::sendMessage(Message& msg)
{
	if (gameSocket)
	{
		gameSocket->sendMessage(msg);
	}
}

void KBEGameSocket::releaseSocket()
{
	if (gameSocket)
	{
		gameSocket->close();
		gameSocket->release();
		gameSocket = NULL;
	}
}

void KBEGameSocket::createSocket()
{
	gameSocket = new GameNetClient();
	gameSocket->autorelease();
	gameSocket->retain();
}

void KBEGameSocket::sendData( char* data ,int size)
{
	if (gameSocket)
		{
			gameSocket->sendData(data,size);
		}
}
