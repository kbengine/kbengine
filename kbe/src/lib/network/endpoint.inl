// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 
namespace Network
{

INLINE EndPoint::EndPoint(u_int32_t networkAddr, u_int16_t networkPort):
#if KBE_PLATFORM == PLATFORM_WIN32
socket_(INVALID_SOCKET)
#else
socket_(-1)
#endif
{
	if(networkAddr)
	{
		address_.ip = networkAddr;
		address_.port = networkPort;
	}

	isRefSocket_ = false;

	sslHandle_ = NULL;
	sslContext_ = NULL;
}

INLINE EndPoint::EndPoint(Address address):
#if KBE_PLATFORM == PLATFORM_WIN32
socket_(INVALID_SOCKET)
#else
socket_(-1)
#endif
{
	if(address.ip > 0)
	{
		address_ = address;
	}

	isRefSocket_ = false;

	sslHandle_ = NULL;
	sslContext_ = NULL;
}

INLINE EndPoint::~EndPoint()
{
	this->close();
}

uint32 EndPoint::getRTT()
{
#if KBE_PLATFORM != PLATFORM_WIN32
	struct tcp_info tcpinfo;
	socklen_t len = sizeof(tcpinfo);

	if (getsockopt((*this), SOL_TCP, TCP_INFO, &tcpinfo, &len) != -1)
		return tcpinfo.tcpi_rtt;
#endif

	return 0;
}

INLINE bool EndPoint::good() const
{
#if KBE_PLATFORM == PLATFORM_WIN32
	return socket_ != INVALID_SOCKET;
#else
	return socket_ != -1;
#endif
}

INLINE EndPoint::operator KBESOCKET() const
{
	return socket_;
}

INLINE KBESOCKET EndPoint::socket() const
{
	return socket_;
}

INLINE void EndPoint::setFileDescriptor(int fd)
{
	socket_ = fd;
}

INLINE void EndPoint::socket(int type)
{
	this->setFileDescriptor((int)::socket(AF_INET, type, 0));

#if KBE_PLATFORM == PLATFORM_WIN32
	if ((socket_ == INVALID_SOCKET) && (WSAGetLastError() == WSANOTINITIALISED))
	{
		EndPoint::initNetwork();
		this->setFileDescriptor((int)::socket(AF_INET, type, 0));
		KBE_ASSERT((socket_ != INVALID_SOCKET) && (WSAGetLastError() != WSANOTINITIALISED) && \
				"EndPoint::socket: create socket error!");
	}
#endif
}

INLINE int EndPoint::setnodelay(bool nodelay)
{
	int arg = int(nodelay);
	return setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char*)&arg, sizeof(int));
}

INLINE int EndPoint::setnonblocking(bool nonblocking)
{
#if KBE_PLATFORM == PLATFORM_UNIX
	int val = nonblocking ? O_NONBLOCK : 0;
	return ::fcntl(socket_, F_SETFL, val);
#else
	u_long val = nonblocking ? 1 : 0;
	return ::ioctlsocket(socket_, FIONBIO, &val);
#endif
}

INLINE int EndPoint::setbroadcast(bool broadcast)
{
#if KBE_PLATFORM == PLATFORM_UNIX
	int val;
	if (broadcast)
	{
		val = 2;
		::setsockopt(socket_, SOL_IP, IP_MULTICAST_TTL, &val, sizeof(int));
	}
#else
	bool val;
#endif
	val = broadcast ? 1 : 0;
	return ::setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, (char*)&val, sizeof(val));
}

INLINE int EndPoint::setreuseaddr(bool reuseaddr)
{
#if KBE_PLATFORM == PLATFORM_UNIX
	int val;
#else
	bool val;
#endif
	val = reuseaddr ? 1 : 0;
	return ::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
		(char*)&val, sizeof(val));
}

INLINE int EndPoint::setlinger(uint16 onoff, uint16 linger)
{
	struct linger l = { 0 };
	l.l_onoff = onoff;
	l.l_linger = linger;
	return setsockopt(socket_, SOL_SOCKET, SO_LINGER, (const char *) &l, sizeof(l));
}

INLINE int EndPoint::setkeepalive(bool keepalive)
{
#if KBE_PLATFORM == PLATFORM_UNIX
	int val;
#else
	bool val;
#endif
	val = keepalive ? 1 : 0;
	return ::setsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE,
		(char*)&val, sizeof(val));
}

INLINE int EndPoint::bind(u_int16_t networkPort, u_int32_t networkAddr)
{
	sockaddr_in	sin;
	memset(&sin, 0, sizeof(sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;
	return ::bind(socket_, (struct sockaddr*)&sin, sizeof(sin));
}

INLINE int EndPoint::joinMulticastGroup(u_int32_t networkAddr)
{
#if KBE_PLATFORM == PLATFORM_UNIX
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt(socket_, SOL_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req));
#else
	return -1;
#endif
}

INLINE int EndPoint::quitMulticastGroup(u_int32_t networkAddr)
{
#if KBE_PLATFORM == PLATFORM_UNIX
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt(socket_, SOL_IP, IP_DROP_MEMBERSHIP,&req, sizeof(req));
#else
	return -1;
#endif
}

INLINE int EndPoint::close()
{
	destroySSL();

	if (socket_ == -1)
	{
		return 0;
	}

	// UDP模式下， socket是服务器listen的fd
	// socket为引用模式
	if (isRefSocket_)
	{
		this->setFileDescriptor(-1);
		return 0;
	}

#if KBE_PLATFORM == PLATFORM_UNIX
	int ret = ::close(socket_);
#else
	int ret = ::closesocket(socket_);
#endif

	if (ret == 0)
	{
		this->setFileDescriptor(-1);
	}

	return ret;
}

INLINE int EndPoint::getlocaladdress(
	u_int16_t * networkPort, u_int32_t * networkAddr) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);

	int ret = ::getsockname(socket_, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}

	return ret;
}

INLINE int EndPoint::getremoteaddress(
	u_int16_t * networkPort, u_int32_t * networkAddr) const
{
	sockaddr_in sin;
	socklen_t sinLen = sizeof(sin);

	int ret = ::getpeername(socket_, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}

	return ret;
}

INLINE const char * EndPoint::c_str() const
{
	return address_.c_str();
}

INLINE const Address& EndPoint::addr() const
{
	return address_;
}

INLINE void EndPoint::addr(const Address& newAddress)
{
	address_ = newAddress;
}

INLINE void EndPoint::addr(u_int16_t newNetworkPort, u_int32_t newNetworkAddress)
{
	address_.port = newNetworkPort;
	address_.ip = newNetworkAddress;
}

INLINE int EndPoint::getremotehostname(std::string * host) const
{
	sockaddr_in sin;
	socklen_t sinLen = sizeof(sin);
	
	int ret = ::getpeername(socket_, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		hostent* h = gethostbyaddr((char*) &sin.sin_addr,
				sizeof(sin.sin_addr), AF_INET);

		if (h)
		{
			*host = h->h_name;
		}
		else
		{
			ret = -1;
		}
	}

	return ret;
}

INLINE int EndPoint::sendto(void * gramData, int gramSize,
	u_int16_t networkPort, u_int32_t networkAddr)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	return this->sendto(gramData, gramSize, sin);
}

INLINE int EndPoint::sendto(void * gramData, int gramSize)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = address_.port;
	sin.sin_addr.s_addr = address_.ip;

	return ::sendto(socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, sizeof(sin));
}

INLINE int EndPoint::sendto(void * gramData, int gramSize,
	struct sockaddr_in & sin)
{
	return ::sendto(socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, sizeof(sin));
}

INLINE int EndPoint::recvfrom(void * gramData, int gramSize,
	u_int16_t * networkPort, u_int32_t * networkAddr)
{
	sockaddr_in sin;
	int result = this->recvfrom(gramData, gramSize, sin);

	if (result >= 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}

	return result;
}

INLINE int EndPoint::recvfrom(void * gramData, int gramSize,
	struct sockaddr_in & sin)
{
	socklen_t		sinLen = sizeof(sin);
	int ret = ::recvfrom(socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, &sinLen);

	return ret;
}

INLINE int EndPoint::listen(int backlog)
{
	return ::listen(socket_, backlog);
}

INLINE int EndPoint::connect(bool autosetflags)
{
	return connect(address_.port, address_.ip, autosetflags);
}

INLINE int EndPoint::connect(u_int16_t networkPort, u_int32_t networkAddr, bool autosetflags)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	int ret = ::connect(socket_, (sockaddr*)&sin, sizeof(sin));
	if(autosetflags)
	{
		setnonblocking(true);
		setnodelay(true);
	}
	
	return ret;
}

INLINE EndPoint * EndPoint::accept(u_int16_t * networkPort, u_int32_t * networkAddr, bool autosetflags)
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = (int)::accept(socket_, (sockaddr*)&sin, &sinLen);

#if KBE_PLATFORM == PLATFORM_UNIX
	if (ret < 0) return NULL;
#else
	if (ret == INVALID_SOCKET) return NULL;
#endif

	EndPoint * pNew = EndPoint::createPoolObject(OBJECTPOOL_POINT);

	pNew->setFileDescriptor(ret);
	pNew->addr(sin.sin_port, sin.sin_addr.s_addr);
	
	if(autosetflags)
	{
		pNew->setnonblocking(true);
		pNew->setnodelay(true);
	}
	
	if (networkPort != NULL) *networkPort = sin.sin_port;
	if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;

	return pNew;
}

INLINE int EndPoint::send(const void * gramData, int gramSize)
{
	if (isSSL())
		return SSL_write(sslHandle_, (char*)gramData, gramSize);

	return ::send(socket_, (char*)gramData, gramSize, 0);
}

INLINE int EndPoint::recv(void * gramData, int gramSize)
{
	if (isSSL())
		return SSL_read(sslHandle_, (char*)gramData, gramSize);

	return ::recv(socket_, (char*)gramData, gramSize, 0);
}

#if KBE_PLATFORM == PLATFORM_UNIX
INLINE int EndPoint::getInterfaceFlags(char * name, int & flags)
{
	struct ifreq	request;

	strncpy(request.ifr_name, name, IFNAMSIZ);
	if (ioctl(socket_, SIOCGIFFLAGS, &request) != 0)
	{
		return -1;
	}

	flags = request.ifr_flags;
	return 0;
}

INLINE int EndPoint::getInterfaceAddress(const char * name, u_int32_t & address)
{
	struct ifreq	request;

	strncpy(request.ifr_name, name, IFNAMSIZ);
	if (ioctl(socket_, SIOCGIFADDR, &request) != 0)
	{
		return -1;
	}

	if (request.ifr_addr.sa_family == AF_INET)
	{
		address = ((sockaddr_in*)&request.ifr_addr)->sin_addr.s_addr;
		return 0;
	}
	else
	{
		return -1;
	}
}

INLINE int EndPoint::getInterfaceNetmask(const char * name,
	u_int32_t & netmask)
{
	struct ifreq request;
	strncpy(request.ifr_name, name, IFNAMSIZ);

	if (ioctl(socket_, SIOCGIFNETMASK, &request) != 0)
	{
		return -1;
	}

	netmask = ((sockaddr_in&)request.ifr_netmask).sin_addr.s_addr;

	return 0;
}

#else
INLINE int EndPoint::getInterfaceFlags(char * name, int & flags)
{
	if (!strcmp(name,"eth0"))
	{
		flags = IFF_UP | IFF_BROADCAST | IFF_NOTRAILERS |
			IFF_RUNNING | IFF_MULTICAST;
		return 0;
	}
	else if (!strcmp(name,"lo"))
	{
		flags = IFF_UP | IFF_LOOPBACK | IFF_RUNNING;
		return 0;
	}
	return -1;
}

INLINE int EndPoint::getInterfaceAddress(const char * name, u_int32_t & address)
{
	if (!strcmp(name, "eth0"))
	{
		char	myname[256];
		::gethostname(myname, sizeof(myname));

		struct hostent * myhost = gethostbyname(myname);
		if (!myhost)
		{
			return -1;
		}

		address = ((struct in_addr*)(myhost->h_addr_list[0]))->s_addr;
		return 0;
	}
	else if (!strcmp(name, "lo"))
	{
		address = htonl(0x7F000001);
		return 0;
	}

	return -1;
}
#endif

}
}
