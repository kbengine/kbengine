/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __KBEENDPOINT__
#define __KBEENDPOINT__

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "network/common.hpp"

namespace KBEngine { 
namespace Mercury
{

class EndPoint
{
public:
	EndPoint(u_int32_t networkAddr = 0, u_int16_t networkPort = 0);
	virtual ~EndPoint();
	operator KBESOCKET() const;
	
	static void initNetwork();
	bool good() const;
		
	void socket(int type);
	KBESOCKET socket() const;
	
	void setFileDescriptor(int fd);

	int joinMulticastGroup(u_int32_t networkAddr);
	int quitMulticastGroup(u_int32_t networkAddr);
	
	INLINE int close();
	INLINE int detach();
	
	int setnonblocking(bool nonblocking);
	int setbroadcast(bool broadcast);
	int setreuseaddr(bool reuseaddr);
	int setkeepalive(bool keepalive);
	int setnodelay(bool nodelay = true);
	
	int bind(u_int16_t networkPort = 0, u_int32_t networkAddr = INADDR_ANY);
	int listen(int backlog = 5);
	int connect(u_int16_t networkPort, u_int32_t networkAddr = INADDR_BROADCAST);
	EndPoint* accept(u_int16_t * networkPort = NULL, u_int32_t * networkAddr = NULL);
	
	INLINE int send(const void * gramData, int gramSize);
	int recv(void * gramData, int gramSize);
	bool recvAll(void * gramData, int gramSize);
	
	int getInterfaceFlags(char * name, int & flags);
	int getInterfaceAddress(const char * name, u_int32_t & address);
	int getInterfaceNetmask(const char * name, u_int32_t & netmask);
	bool getInterfaces(std::map< u_int32_t, std::string > &interfaces);
	int findDefaultInterface(char * name);
	int findIndicatedInterface(const char * spec, char * name);
	static int convertAddress(const char * string, u_int32_t & address);
	
	int transmitQueueSize() const;
	int receiveQueueSize() const;
	int getQueueSizes(int & tx, int & rx) const;

	int getBufferSize(int optname) const;
	bool setBufferSize(int optname, int size);
	
	int getlocaladdress(u_int16_t * networkPort, u_int32_t * networkAddr) const;
	int getremoteaddress(u_int16_t * networkPort, u_int32_t * networkAddr) const;
	
	Mercury::Address getLocalAddress() const;
	Mercury::Address getRemoteAddress() const;

	const char * c_str() const;
	int getremotehostname(std::string * name)const;

	bool getClosedPort(Mercury::Address & closedPort);
	
	int sendto(void * gramData, int gramSize, u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);
	INLINE int sendto(void * gramData, int gramSize, struct sockaddr_in & sin);
	INLINE int recvfrom(void * gramData, int gramSize, u_int16_t * networkPort, u_int32_t * networkAddr);
	INLINE int recvfrom(void * gramData, int gramSize, struct sockaddr_in & sin);
	
	INLINE const Address& addr() const;
	void addr(const Address& newAddress);
	void addr(u_int16_t port, u_int32_t newAddress);
protected:
	KBESOCKET	socket_;
	Address address_;
};

}
}

#ifdef CODE_INLINE
#include "endpoint.ipp"
#endif
#endif // __KBEENDPOINT__