/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __KBESOCKET__
#define __KBESOCKET__

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"

namespace KBEngine { 
namespace Mercury
{

class Socket
{
public:
	Socket();
	virtual ~Socket();
	operator int() const;
	static void initNetwork();
	bool valid() const;
		
	void socket(int type);
	void setFileDescriptor(int fd);

	int joinMulticastGroup(uint32 networkAddr);
	int quitMulticastGroup(uint32 networkAddr);
	
	INLINE int close();
	INLINE int detach();
	
	int setnonblocking(bool nonblocking);
	int setbroadcast(bool broadcast);
	int setreuseaddr(bool reuseaddr);
	int setkeepalive(bool keepalive);

	int bind(uint16 networkPort = 0, uint32 networkAddr = INADDR_ANY);
	int listen(int backlog = 5);
	int connect(uint16 networkPort, uint32 networkAddr = INADDR_BROADCAST);
	Socket* accept(uint16 * networkPort = NULL, uint32 * networkAddr = NULL);
	
	INLINE int send(const void * gramData, int gramSize);
	int recv(void * gramData, int gramSize);
	bool recvAll(void * gramData, int gramSize);
	
	int getInterfaceFlags(char * name, int & flags);
	int getInterfaceAddress(const char * name, uint32 & address);
	int getInterfaceNetmask(const char * name, uint32 & netmask);
	bool getInterfaces(std::map< uint32, std::string > &interfaces);
	int findDefaultInterface(char * name);
	int findIndicatedInterface(const char * spec, char * name);
	static int convertAddress(const char * string, uint32 & address);
	
	int transmitQueueSize() const;
	int receiveQueueSize() const;
	int getQueueSizes(int & tx, int & rx) const;

	int getBufferSize(int optname) const;
	bool setBufferSize(int optname, int size);
	
	int getlocaladdress(uint16 * networkPort, uint32 * networkAddr) const;
	int getremoteaddress(uint16 * networkPort, uint32 * networkAddr) const;
	
	Mercury::Address getLocalAddress() const;
	Mercury::Address getRemoteAddress() const;

	const char * c_str() const;
	int getremotehostname(std::string * name)const;

	bool getClosedPort(Mercury::Address & closedPort);
	
	int sendto(void * gramData, int gramSize, uint16 networkPort, uint32 networkAddr = INADDR_BROADCAST);
	INLINE int sendto(void * gramData, int gramSize, struct sockaddr_in & sin);
	INLINE int recvfrom(void * gramData, int gramSize, uint16 * networkPort, uint32 * networkAddr);
	INLINE int recvfrom(void * gramData, int gramSize, struct sockaddr_in & sin);
protected:
#if defined(unix) || defined(PLAYSTATION3)
	int	socket_;
#else //ifdef unix
	SOCKET	socket_;
#endif //def _WIN32
};

}
}

#ifdef CODE_INLINE
#include "socket.ipp"
#endif
#endif // __KBESOCKET__