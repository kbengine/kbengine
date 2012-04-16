namespace KBEngine { 
namespace Mercury
{

INLINE Socket::Socket():
socket_(-1)
{
}

INLINE Socket::~Socket()
{
	this->close();
}

INLINE bool Socket::good() const
{
	return socket_ != -1;
}

INLINE Socket::operator int() const
{
	return socket_;
}

INLINE void Socket::setFileDescriptor(int fd)
{
	socket_ = fd;
}

INLINE void Socket::socket(int type)
{
	this->setFileDescriptor(::socket(PF_INET, type, 0));
#if KBE_PLATFORM == PLATFORM_WIN32
	if ((socket_ == INVALID_SOCKET) && (WSAGetLastError() == WSANOTINITIALISED))
	{
		Socket::initNetwork();
		this->setFileDescriptor(::socket(PF_INET, type, 0));
		KBE_ASSERT((socket_ != INVALID_SOCKET) && (WSAGetLastError() != WSANOTINITIALISED) && \
				"Socket::socket: create socket is error!");
	}
#endif
}

INLINE int Socket::setnodelay(bool nodelay)
{
	int arg = int(nodelay);
	return setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char*)&arg, sizeof(int));
}

INLINE int Socket::setnonblocking(bool nonblocking)
{
#ifdef unix
	int val = nonblocking ? O_NONBLOCK : 0;
	return ::fcntl(socket_, F_SETFL, val);
#elif defined(PLAYSTATION3)
	int val = nonblocking ? 1 : 0;
	return setsockopt(socket_, SOL_SOCKET, SO_NBIO, &val, sizeof(int));
#else
	u_long val = nonblocking ? 1 : 0;
	return ::ioctlsocket(socket_, FIONBIO, &val);
#endif
}

INLINE int Socket::setbroadcast(bool broadcast)
{
#ifdef unix
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

INLINE int Socket::setreuseaddr(bool reuseaddr)
{
#ifdef unix
	int val;
#else
	bool val;
#endif
	val = reuseaddr ? 1 : 0;
	return ::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
		(char*)&val, sizeof(val));
}

INLINE int Socket::setkeepalive(bool keepalive)
{
#ifdef unix
	int val;
#else
	bool val;
#endif
	val = keepalive ? 1 : 0;
	return ::setsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE,
		(char*)&val, sizeof(val));
}

INLINE int Socket::bind(uint16 networkPort, uint32 networkAddr)
{
	sockaddr_in	sin;
	memset(&sin, 0, sizeof(sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;
	return ::bind(socket_, (struct sockaddr*)&sin, sizeof(sin));
}

INLINE int Socket::joinMulticastGroup(uint32 networkAddr)
{
#ifdef unix
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt(socket_, SOL_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req));
#else
	return -1;
#endif
}

INLINE int Socket::quitMulticastGroup(uint32 networkAddr)
{
#ifdef unix
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt(socket_, SOL_IP, IP_DROP_MEMBERSHIP,&req, sizeof(req));
#else
	return -1;
#endif
}

INLINE int Socket::close()
{
	if (socket_ == -1)
	{
		return 0;
	}

#ifdef unix
	int ret = ::close(socket_);
#elif defined(PLAYSTATION3)
	int ret = ::socketclose(socket_);
#else
	int ret = ::closesocket(socket_);
#endif
	if (ret == 0)
	{
		this->setFileDescriptor(-1);
	}
	return ret;
}

INLINE int Socket::detach()
{
	int ret = socket_;
	this->setFileDescriptor(-1);
	return ret;
}

INLINE int Socket::getlocaladdress(
	uint16 * networkPort, uint32 * networkAddr) const
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

INLINE int Socket::getremoteaddress(
	uint16 * networkPort, uint32 * networkAddr) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::getpeername(socket_, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}
	return ret;
}

INLINE const char * Socket::c_str() const
{
	Mercury::Address addr;
	this->getlocaladdress(&(uint16&)addr.port, &(uint32&)addr.ip);
	return addr.c_str();
}

INLINE int Socket::getremotehostname(std::string * host) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
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

INLINE int Socket::sendto(void * gramData, int gramSize,
	uint16 networkPort, uint32 networkAddr)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	return this->sendto(gramData, gramSize, sin);
}

INLINE int Socket::sendto(void * gramData, int gramSize,
	struct sockaddr_in & sin)
{
	return ::sendto(socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, sizeof(sin));
}

INLINE int Socket::recvfrom(void * gramData, int gramSize,
	uint16 * networkPort, uint32 * networkAddr)
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

INLINE int Socket::recvfrom(void * gramData, int gramSize,
	struct sockaddr_in & sin)
{
	socklen_t		sinLen = sizeof(sin);
	int ret = ::recvfrom(socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, &sinLen);

	return ret;
}

INLINE int Socket::listen(int backlog)
{
	return ::listen(socket_, backlog);
}

INLINE int Socket::connect(uint16 networkPort, uint32 networkAddr)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	return ::connect(socket_, (sockaddr*)&sin, sizeof(sin));
}

INLINE Socket * Socket::accept(uint16 * networkPort, uint32 * networkAddr)
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::accept(socket_, (sockaddr*)&sin, &sinLen);
#if defined(unix) || defined(PLAYSTATION3)
	if (ret < 0) return NULL;
#else
	if (ret == INVALID_SOCKET) return NULL;
#endif

	Socket * pNew = new Socket();
	pNew->setFileDescriptor(ret);

	if (networkPort != NULL) *networkPort = sin.sin_port;
	if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;

	return pNew;
}

INLINE int Socket::send(const void * gramData, int gramSize)
{
	return ::send(socket_, (char*)gramData, gramSize, 0);
}

INLINE int Socket::recv(void * gramData, int gramSize)
{
	return ::recv(socket_, (char*)gramData, gramSize, 0);
}


#ifdef unix
INLINE int Socket::getInterfaceFlags(char * name, int & flags)
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

INLINE int Socket::getInterfaceAddress(const char * name, uint32 & address)
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

INLINE int Socket::getInterfaceNetmask(const char * name,
	uint32 & netmask)
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
INLINE int Socket::getInterfaceFlags(char * name, int & flags)
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

INLINE int Socket::getInterfaceAddress(const char * name, uint32 & address)
{
	if (!strcmp(name,"eth0"))
	{
#if defined(PLAYSTATION3)
		CellNetCtlInfo netInfo;
		int ret = cellNetCtlGetInfo(CELL_NET_CTL_INFO_IP_ADDRESS, &netInfo);
		MF_ASSERT(ret == 0);
		int ip0, ip1, ip2, ip3;
		sscanf(netInfo.ip_address, "%d.%d.%d.%d", &ip0, &ip1, &ip2, &ip3);
		address = (ip0 << 24) | (ip1 << 16) | (ip2 << 8) | (ip3 << 0);
#else
		char	myname[256];
		::gethostname(myname,sizeof(myname));

		struct hostent * myhost = gethostbyname(myname);
		if (!myhost)
		{
			return -1;
		}

		address = ((struct in_addr*)(myhost->h_addr_list[0]))->s_addr;
#endif
		return 0;
	}
	else if (!strcmp(name,"lo"))
	{
		address = htonl(0x7F000001);
		return 0;
	}

	return -1;
}
#endif

INLINE int Socket::transmitQueueSize() const
{
	int tx=0, rx=0;
	this->getQueueSizes(tx, rx);
	return tx;
}

INLINE int Socket::receiveQueueSize() const
{
	int tx=0, rx=0;
	this->getQueueSizes(tx, rx);
	return rx;
}

}
}
