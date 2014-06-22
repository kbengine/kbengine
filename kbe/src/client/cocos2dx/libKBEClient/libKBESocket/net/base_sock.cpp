//---------------------------------------------------------------------------
#ifdef WIN32
#include <winsock2.h>
#endif
#include <errno.h>
#include "base_sock.h"
#ifndef WIN32
#define strnicmp strncasecmp
#endif

NS_GC_BEGIN

bool InitSocketSystem()
{
#ifdef WIN32
	WSADATA wsadata;
	unsigned short winsock_version=MAKEWORD( 1, 1 );
	if(WSAStartup(winsock_version,&wsadata))
	{
		return false;
	}
#endif
	return true;
}

void clearSocketSystem()
{
#ifdef WIN32
	WSACleanup();
#endif
}
BaseSock::BaseSock()
{
	m_bUDP = false;
	m_sock = -1;
	m_nPort = 0;
	m_bConnected = false;
}
BaseSock::~BaseSock()
{
	Close();
}
SOCKET BaseSock::GetHandle()
{
	return m_sock;
}
void BaseSock::Close()
{
	if (m_sock != -1)
	{
#ifdef WIN32
		shutdown(m_sock,SD_BOTH);
		closesocket(m_sock);
#else
		shutdown(m_sock, SHUT_RDWR);
		close(m_sock);
#endif
		m_sock = -1;
	}
	m_bConnected = false;
}
bool BaseSock::Create(bool bUDP)
{
	m_bUDP = bUDP;
	if (!m_bUDP)
		m_sock = socket(AF_INET, SOCK_STREAM, 0);
	else
	{
		m_sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (m_sock != -1)
		{
			m_bConnected = true;
		}
	}
	//SetBlock();
	return (m_sock != -1);
}
bool BaseSock::Connect(const std::string& host, unsigned short port)
{
	if (m_sock == -1)
		return false;
	m_strHost = host;
	m_nPort = port;
	struct hostent * he = gethostbyname(host.c_str());
	if (he == NULL)
	{
		Close();
		return false;
	}
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr = *((struct in_addr*) he->h_addr);
	memset(sin.sin_zero, 0, 8);
	sin.sin_port = htons(port);
	if (connect(m_sock, (struct sockaddr *) &sin, sizeof(sin)))
	{
		Close();
		return false;
	}
	unsigned long rb=1;
	//ioctlsocket(m_sock,FIONBIO,&rb);
#ifdef WIN32
		ioctlsocket(m_sock, FIONBIO, &rb);
#else
		ioctl(m_sock, FIONBIO, &rb);
#endif
	int bufsize = MAX_RECV_BUFFERSIZE;
	setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char*) &bufsize, sizeof(bufsize));
	setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, sizeof(bufsize));
	//int timeout= 20000;
	//setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	m_bConnected = true;
	return true;
}
long BaseSock::Send(const char* buf, long buflen)
{
	//printf("Send: %.*s\r\n",buflen,buf);
	if (m_sock == -1)
	{
		return -1;
	}

	int sended = 0;
	do
	{
		int len = send(m_sock, buf + sended, buflen - sended, 0);
		if (len < 0)
		{
			break;
		}
		sended += len;
	} while (sended < buflen);
	return sended;
}
long BaseSock::Recv(char* buf, long buflen)
{
	if (m_sock == -1)
	{
		return -1;
	}
	/*fd_set fd;
	FD_ZERO(&fd);
	FD_SET(m_sock, &fd);
	struct timeval val =
	{ SOCKET_TIMEOUT, 0 };
	int selret = select(m_sock + 1, &fd, NULL, NULL, &val);
	if (selret <= 0)
	{
		return selret;
	}*/
	int len = recv(m_sock, buf, buflen, 0);
	//printf("Recv: %.*s\r\n",len,buf);
	return len;
}
bool BaseSock::GetPeerName(std::string& strIP, unsigned short &nPort)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	int addrlen = sizeof(addr);
#ifdef WIN32
	if(getpeername(m_sock,(struct sockaddr*)&addr,&addrlen)!=0)
#else
	if (getpeername(m_sock, (struct sockaddr*) &addr, (socklen_t*) &addrlen)
			!= 0)
#endif
		return false;
	char szIP[64];
#ifdef WIN32
	sprintf(szIP,"%u.%u.%u.%u",addr.sin_addr.S_un.S_addr&0xFF,(addr.sin_addr.S_un.S_addr>>8)&0xFF,(addr.sin_addr.S_un.S_addr>>16)&0xFF,(addr.sin_addr.S_un.S_addr>>24)&0xFF);
#else
	sprintf(szIP, "%u.%u.%u.%u", addr.sin_addr.s_addr & 0xFF,
			(addr.sin_addr.s_addr >> 8) & 0xFF, (addr.sin_addr.s_addr >> 16)
					& 0xFF, (addr.sin_addr.s_addr >> 24) & 0xFF);
#endif
	strIP = szIP;
	nPort = ntohs(addr.sin_port);
	return true;
}
bool BaseSock::isConnected()
{
	return (m_sock != -1) && m_bConnected;
}
bool BaseSock::isUDP()
{
	return m_bUDP;
}
long BaseSock::SendTo(const char* buf, int len,
		const struct sockaddr_in* toaddr, int tolen)
{
	if (m_sock == -1)
	{
		return -1;
	}
	return sendto(m_sock, buf, len, 0, (const struct sockaddr*) toaddr, tolen);
}
long BaseSock::RecvFrom(char* buf, int len, struct sockaddr_in* fromaddr,
		int* fromlen)
{
	if (m_sock == -1)
	{
		return -1;
	}
#ifdef WIN32
	return recvfrom(m_sock,buf,len,0,(struct sockaddr*)fromaddr,fromlen);
#else
	return recvfrom(m_sock, buf, len, 0, (struct sockaddr*) fromaddr,
			(socklen_t*) fromlen);
#endif
}
bool BaseSock::Bind(unsigned short nPort)
{
	if (m_sock == -1)
	{
		return false;
	}
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
#ifdef WIN32
	sin.sin_addr.S_un.S_addr=0;
#else
	sin.sin_addr.s_addr = 0;
#endif
	memset(sin.sin_zero, 0, 8);
	sin.sin_port = htons(nPort);
	if (::bind(m_sock, (sockaddr*) &sin, sizeof(sockaddr_in)) != 0)
		return false;
	listen(m_sock, 1024);
	m_bConnected = true;
	return true;
}
bool BaseSock::Accept(BaseSock& client)
{
	if (m_sock == -1)
	{
		return false;
	}
	client.m_sock = accept(m_sock, NULL, NULL);
	client.m_bConnected = true;
	return (client.m_sock != -1);
}

int BaseSock::SetBlock(bool bBlock)
{
	int bufsize = MAX_RECV_BUFFERSIZE;
	setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(bufsize));
	setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, sizeof(bufsize));

	int rt = -1;
	if(bBlock)
	{
		unsigned long rb = 1;
#ifdef WIN32
		rt = ioctlsocket(m_sock, FIONBIO, &rb);
#else
		rt = ioctl(m_sock, FIONBIO, &rb);
#endif
	}
	return rt;
}

NS_GC_END