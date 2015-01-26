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


#include "endpoint.h"
#ifndef CODE_INLINE
#include "endpoint.inl"
#endif

namespace KBEngine { 
namespace Network
{
#ifdef unix
#else	// not unix
	// Need to implement if_nameindex functions on Windows
	/** @internal */
	struct if_nameindex
	{

		unsigned int if_index;	/* 1, 2, ... */

		char *if_name;			/* null terminated name: "eth0", ... */

	};

	/** @internal */
	struct if_nameindex *if_nameindex(void)
	{
		static struct if_nameindex staticIfList[3] =
		{ { 1, "eth0" }, { 2, "lo" }, { 0, 0 } };

		return staticIfList;
	}

	/** @internal */
	inline void if_freenameindex(struct if_nameindex *)
	{}
#endif	// not unix

static bool g_networkInitted = false;

//-------------------------------------------------------------------------------------
static ObjectPool<EndPoint> _g_objPool("EndPoint");
ObjectPool<EndPoint>& EndPoint::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
void EndPoint::destroyObjPool()
{
	DEBUG_MSG(fmt::format("EndPoint::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
EndPoint::SmartPoolObjectPtr EndPoint::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<EndPoint>(ObjPool().createObject(), _g_objPool));
}

//-------------------------------------------------------------------------------------
void EndPoint::onReclaimObject()
{
#if KBE_PLATFORM == PLATFORM_WIN32
	socket_ = INVALID_SOCKET;
#else
	socket_ = -1;
#endif

	address_ = Address::NONE;
}

//-------------------------------------------------------------------------------------
bool EndPoint::getClosedPort(Network::Address & closedPort)
{
	bool isResultSet = false;

#ifdef unix
//	KBE_ASSERT(errno == ECONNREFUSED);

	struct sockaddr_in	offender;
	offender.sin_family = 0;
	offender.sin_port = 0;
	offender.sin_addr.s_addr = 0;

	struct msghdr	errHeader;
	struct iovec	errPacket;

	char data[ 256 ];
	char control[ 256 ];

	errHeader.msg_name = &offender;
	errHeader.msg_namelen = sizeof(offender);
	errHeader.msg_iov = &errPacket;
	errHeader.msg_iovlen = 1;
	errHeader.msg_control = control;
	errHeader.msg_controllen = sizeof(control);
	errHeader.msg_flags = 0;	// result only

	errPacket.iov_base = data;
	errPacket.iov_len = sizeof(data);

	int errMsgErr = recvmsg(*this, &errHeader, MSG_ERRQUEUE);
	if (errMsgErr < 0)
	{
		return false;
	}

	struct cmsghdr * ctlHeader;

	for (ctlHeader = CMSG_FIRSTHDR(&errHeader);
		ctlHeader != NULL;
		ctlHeader = CMSG_NXTHDR(&errHeader,ctlHeader))
	{
		if (ctlHeader->cmsg_level == SOL_IP &&
			ctlHeader->cmsg_type == IP_RECVERR) break;
	}

	// Was there an IP_RECVERR error.

	if (ctlHeader != NULL)
	{
		struct sock_extended_err * extError =
			(struct sock_extended_err*)CMSG_DATA(ctlHeader);

		// Only use this address if the kernel has the bug where it does not
		// report the packet details.

		if (errHeader.msg_namelen == 0)
		{
			// Finally we figure out whose fault it is except that this is the
			// generator of the error (possibly a machine on the path to the
			// destination), and we are interested in the actual destination.
			offender = *(sockaddr_in*)SO_EE_OFFENDER(extError);
			offender.sin_port = 0;

			ERROR_MSG("EndPoint::getClosedPort: "
				"Kernel has a bug: recv_msg did not set msg_name.\n");
		}

		closedPort.ip = offender.sin_addr.s_addr;
		closedPort.port = offender.sin_port;

		isResultSet = true;
	}
#endif // unix

	return isResultSet;
}
//-------------------------------------------------------------------------------------
bool EndPoint::getInterfaces(std::map< u_int32_t, std::string > &interfaces)
{
#ifdef _WIN32
	int count = 0;
	char hostname[1024];
	struct hostent* inaddrs;

	if(gethostname(hostname, 1024) == 0)
	{
		inaddrs = gethostbyname(hostname);
		if(inaddrs)
		{
			while(inaddrs->h_addr_list[count])
			{
				unsigned long addrs = *(unsigned long*)inaddrs->h_addr_list[count];
				interfaces[addrs] = "eth0";
				char *ip = inet_ntoa (*(struct in_addr *)inaddrs->h_addr_list[count]);
				DEBUG_MSG(fmt::format("EndPoint::getInterfaces: found eth0 {}\n", ip));
				++count;
			}
		}
	}

	return count > 0;
#else
	struct ifconf ifc;
	char          buf[1024];

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;

	if(ioctl(socket_, SIOCGIFCONF, &ifc) < 0)
	{
		ERROR_MSG("EndPoint::getInterfaces: ioctl(SIOCGIFCONF) failed.\n");
		return false;
	}

	// Iterate through the list of interfaces.
	struct ifreq * ifr         = ifc.ifc_req;
	int nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
	for (int i = 0; i < nInterfaces; ++i)
	{
		struct ifreq *item = &ifr[i];

		interfaces[ ((struct sockaddr_in *)&item->ifr_addr)->sin_addr.s_addr ] =
			item->ifr_name;
	}

	return true;
#endif
}

//-------------------------------------------------------------------------------------
int EndPoint::findDefaultInterface(char * name)
{
#ifndef unix
	strcpy(name, "eth0");
	return 0;
#else
	int		ret = -1;

	struct if_nameindex* pIfInfo = if_nameindex();
	if (pIfInfo)
	{
		int		flags = 0;
		struct if_nameindex* pIfInfoCur = pIfInfo;
		while (pIfInfoCur->if_name)
		{
			flags = 0;
			this->getInterfaceFlags(pIfInfoCur->if_name, flags);

			if ((flags & IFF_UP) && (flags & IFF_RUNNING))
			{
				u_int32_t	addr;
				if (this->getInterfaceAddress(pIfInfoCur->if_name, addr) == 0)
				{
					strcpy(name, pIfInfoCur->if_name);
					ret = 0;

					// we only stop if it's not a loopback address,
					// otherwise we continue, hoping to find a better one
					if (!(flags & IFF_LOOPBACK)) break;
				}
			}
			++pIfInfoCur;
		}
		if_freenameindex(pIfInfo);
	}
	else
	{
		ERROR_MSG(fmt::format("EndPoint::findDefaultInterface: "
							"if_nameindex returned NULL ({})\n",
						kbe_strerror()));
	}

	return ret;
#endif // unix
}

//-------------------------------------------------------------------------------------
int EndPoint::findIndicatedInterface(const char * spec, char * name)
{
	name[0] = 0;

	if (spec == NULL || spec[0] == 0) 
		return -1;

	int netmaskbits = 32;
	char iftemp[IFNAMSIZ+16];

	strncpy(iftemp, spec, IFNAMSIZ); 
	iftemp[IFNAMSIZ] = 0;
	u_int32_t addr = 0;

	if (this->getInterfaceAddress(iftemp, addr) == 0)
	{
		// specified name of interface
		strncpy(name, iftemp, IFNAMSIZ);
	}
	else if (EndPoint::convertAddress(spec, addr) == 0)
	{
		// specified ip address
		netmaskbits = 32; // redundant but instructive
	}
	else
	{
		ERROR_MSG(fmt::format("EndPoint::findIndicatedInterface: "
			"No interface matching interface spec '{}' found\n", spec));

		return -1;
	}

	// if we haven't set a name yet then we're supposed to
	// look up the ip address
	if (name[0] == 0)
	{
		int netmaskshift = 32-netmaskbits;
		u_int32_t netmaskmatch = ntohl(addr);

		std::vector< std::string > interfaceNames;

		struct if_nameindex* pIfInfo = if_nameindex();
		if (pIfInfo)
		{
			struct if_nameindex* pIfInfoCur = pIfInfo;
			while (pIfInfoCur->if_name)
			{
				interfaceNames.push_back(pIfInfoCur->if_name);
				++pIfInfoCur;
			}
			if_freenameindex(pIfInfo);
		}

		std::vector< std::string >::iterator iter = interfaceNames.begin();

		while (iter != interfaceNames.end())
		{
			u_int32_t tip = 0;
			char * currName = (char *)iter->c_str();

			if (this->getInterfaceAddress(currName, tip) == 0)
			{
				u_int32_t htip = ntohl(tip);

				if ((htip >> netmaskshift) == (netmaskmatch >> netmaskshift))
				{
					//DEBUG_MSG("EndPoint::bind(): found a match\n");
					strncpy(name, currName, IFNAMSIZ);
					break;
				}
			}

			++iter;
		}

		if (name[0] == 0)
		{
			uint8 * qik = (uint8*)&addr;
			ERROR_MSG(fmt::format("EndPoint::findIndicatedInterface: "
				"No interface matching netmask spec '{}' found "
				"(evals to {}.{}.{}.{}/{})\n", spec,
				qik[0], qik[1], qik[2], qik[3], netmaskbits));

			return -2; // parsing ok, just didn't match
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------
int EndPoint::convertAddress(const char * string, u_int32_t & address)
{
	u_int32_t	trial;

#ifdef unix
	if (inet_aton(string, (struct in_addr*)&trial) != 0)
#else
	if ((trial = inet_addr(string)) != INADDR_NONE)
#endif
		{
			address = trial;
			return 0;
		}

	struct hostent * hosts = gethostbyname(string);
	if (hosts != NULL)
	{
		address = *(u_int32_t*)(hosts->h_addr_list[0]);
		return 0;
	}

	return -1;
}

//-------------------------------------------------------------------------------------
int EndPoint::getBufferSize(int optname) const
{
	KBE_ASSERT(optname == SO_SNDBUF || optname == SO_RCVBUF);

	int recvbuf = -1;
	socklen_t rbargsize = sizeof(int);

	int rberr = getsockopt(socket_, SOL_SOCKET, optname,
		(char*)&recvbuf, &rbargsize);

	if (rberr == 0 && rbargsize == sizeof(int))
		return recvbuf;

	ERROR_MSG(fmt::format("EndPoint::getBufferSize: "
		"Failed to read option {}: {}\n",
		(optname == SO_SNDBUF ? "SO_SNDBUF" : "SO_RCVBUF"),
		kbe_strerror()));

	return -1;
}

//-------------------------------------------------------------------------------------
bool EndPoint::setBufferSize(int optname, int size)
{
	setsockopt(socket_, SOL_SOCKET, optname, (const char*)&size, sizeof(size));

	return this->getBufferSize(optname) >= size;
}

//-------------------------------------------------------------------------------------
bool EndPoint::recvAll(void * gramData, int gramSize)
{
	while (gramSize > 0)
	{
		int len = this->recv(gramData, gramSize);

		if (len <= 0)
		{
			if (len == 0)
			{
				WARNING_MSG("EndPoint::recvAll: Connection lost\n");
			}
			else
			{
				WARNING_MSG(fmt::format("EndPoint::recvAll: Got error '{}'\n",
					kbe_strerror()));
			}

			return false;
		}
		gramSize -= len;
		gramData = ((char *)gramData) + len;
	}

	return true;
}

//-------------------------------------------------------------------------------------
Network::Address EndPoint::getLocalAddress() const
{
	Network::Address addr(0, 0);

	if (this->getlocaladdress((u_int16_t*)&addr.port,
				(u_int32_t*)&addr.ip) == -1)
	{
		ERROR_MSG("EndPoint::getLocalAddress: Failed\n");
	}

	return addr;
}

//-------------------------------------------------------------------------------------
Network::Address EndPoint::getRemoteAddress() const
{
	Network::Address addr(0, 0);

	if (this->getremoteaddress((u_int16_t*)&addr.port,
				(u_int32_t*)&addr.ip) == -1)
	{
		ERROR_MSG("EndPoint::getRemoteAddress: Failed\n");
	}

	return addr;
}

//-------------------------------------------------------------------------------------
void EndPoint::initNetwork()
{
	if (g_networkInitted) 
		return;
	
	g_networkInitted = true;

#if KBE_PLATFORM == PLATFORM_WIN32
	WSAData wsdata;
	WSAStartup(0x202, &wsdata);
#endif
}

//-------------------------------------------------------------------------------------
bool EndPoint::waitSend()
{
	fd_set	fds;
	struct timeval tv = { 0, 10000 };
	FD_ZERO( &fds );
	FD_SET(socket_, &fds);

	return select(socket_+1, NULL, &fds, NULL, &tv) > 0;
}

//-------------------------------------------------------------------------------------
}
}
