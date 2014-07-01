#pragma once

//win32
#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#else
//for ios and andriod
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#define SOCKET int

#endif

using namespace std;

#include <errno.h>
#include <string>


#include "KBEClientCoreMacros.h"

NS_GC_BEGIN

#define SOCKET_TIMEOUT 30
#define MAX_RECV_BUFFERSIZE 65535

bool InitSocketSystem();
void clearSocketSystem();

class BaseSock
{
public:
	SOCKET m_sock;
public:
	BaseSock();
	virtual ~BaseSock();
	bool Create(bool bUDP = false);
	virtual bool Connect(const std::string& host, unsigned short port);
	virtual bool isConnected();
	virtual bool isUDP();
	virtual bool Bind(unsigned short nPort);
	virtual bool Accept(BaseSock& client);
	virtual void Close();
	virtual long Send(const char* buf, long buflen);
	virtual long Recv(char* buf, long buflen);
	virtual long SendTo(const char* buf, int len,
			const struct sockaddr_in* toaddr, int tolen);
	virtual long RecvFrom(char* buf, int len, struct sockaddr_in* fromaddr,
			int* fromlen);
	virtual bool GetPeerName(std::string& strIP, unsigned short &nPort);
	SOCKET GetHandle();
	int SetBlock(bool bBlock = true);
private:
	bool m_bUDP;
	bool m_bConnected;
	std::string m_strHost;
	unsigned short m_nPort;
};

NS_GC_END