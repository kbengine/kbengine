// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_ENDPOINT_H
#define KBE_ENDPOINT_H

#include "common/common.h"
#include "common/objectpool.h"
#include "helper/debug_helper.h"
#include "network/address.h"
#include "network/common.h"
#include "openssl/ssl.h"

namespace KBEngine { 
namespace Network
{

class Bundle;
class EndPoint : public PoolObject
{
public:
	typedef KBEShared_ptr< SmartPoolObject< EndPoint > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj(const std::string& logPoint);
	static ObjectPool<EndPoint>& ObjPool();
	static EndPoint* createPoolObject(const std::string& logPoint);
	static void reclaimPoolObject(EndPoint* obj);
	static void destroyObjPool();
	void onReclaimObject();

	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(KBESOCKET)
		 + address_.getPoolObjectBytes();

		return bytes;
	}

	EndPoint(Address address);
	EndPoint(u_int32_t networkAddr = 0, u_int16_t networkPort = 0);
	virtual ~EndPoint();

	INLINE operator KBESOCKET() const;
	
	static void initNetwork();
	INLINE bool good() const;
		
	void socket(int type);
	INLINE KBESOCKET socket() const;
	
	INLINE void setFileDescriptor(int fd);

	INLINE int joinMulticastGroup(u_int32_t networkAddr);
	INLINE int quitMulticastGroup(u_int32_t networkAddr);
	
	INLINE int close();
	
	INLINE int setnonblocking(bool nonblocking);
	INLINE int setbroadcast(bool broadcast);
	INLINE int setreuseaddr(bool reuseaddr);
	INLINE int setkeepalive(bool keepalive);
	INLINE int setnodelay(bool nodelay = true);
	INLINE int setlinger(uint16 onoff, uint16 linger);

	INLINE int bind(u_int16_t networkPort = 0, u_int32_t networkAddr = INADDR_ANY);

	INLINE int listen(int backlog = 5);

	INLINE int connect(u_int16_t networkPort, u_int32_t networkAddr = INADDR_BROADCAST, bool autosetflags = true);
	INLINE int connect(bool autosetflags = true);

	INLINE EndPoint* accept(u_int16_t * networkPort = NULL, u_int32_t * networkAddr = NULL, bool autosetflags = true);
	
	INLINE int send(const void * gramData, int gramSize);
	void send(Bundle * pBundle);

	INLINE int recv(void * gramData, int gramSize);
	bool recvAll(void * gramData, int gramSize);
	
	INLINE uint32 getRTT();

	INLINE int getInterfaceFlags(char * name, int & flags);
	INLINE int getInterfaceAddress(const char * name, u_int32_t & address);
	INLINE int getInterfaceNetmask(const char * name, u_int32_t & netmask);
	bool getInterfaces(std::map< u_int32_t, std::string > &interfaces);

	int findIndicatedInterface(const char * spec, u_int32_t & address);
	int findDefaultInterface(char * name, int buffsize);

	int getInterfaceAddressByName(const char * name, u_int32_t & address);
	int getInterfaceAddressByMAC(const char * mac, u_int32_t & address);
	int getDefaultInterfaceAddress(u_int32_t & address);

	int getBufferSize(int optname) const;
	bool setBufferSize(int optname, int size);
	
	INLINE int getlocaladdress(u_int16_t * networkPort, u_int32_t * networkAddr) const;
	INLINE int getremoteaddress(u_int16_t * networkPort, u_int32_t * networkAddr) const;
	
	Network::Address getLocalAddress() const;
	Network::Address getRemoteAddress() const;

	bool getClosedPort(Network::Address & closedPort);

	INLINE const char * c_str() const;
	INLINE int getremotehostname(std::string * name) const;
	
	INLINE int sendto(void * gramData, int gramSize);
	INLINE int sendto(void * gramData, int gramSize, u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);
	INLINE int sendto(void * gramData, int gramSize, struct sockaddr_in & sin);
	void sendto(Bundle * pBundle, u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);

	INLINE int recvfrom(void * gramData, int gramSize, u_int16_t * networkPort, u_int32_t * networkAddr);
	INLINE int recvfrom(void * gramData, int gramSize, struct sockaddr_in & sin);
	
	INLINE const Address& addr() const;
	INLINE void addr(const Address& newAddress);
	INLINE void addr(u_int16_t newNetworkPort, u_int32_t newNetworkAddress);

	bool waitSend();

	void setSocketRef(KBESOCKET s) 
	{
		socket_ = s;
		isRefSocket_ = true;
	}

	bool setupSSL(int sslVersion, Packet* pPacket);
	bool destroySSL();

	bool isSSL() const {
		return sslHandle_ != NULL;
	}

protected:
	KBESOCKET socket_;
	Address address_;
	SSL* sslHandle_;
	SSL_CTX* sslContext_;
	bool isRefSocket_;
};

}
}

#ifdef CODE_INLINE
#include "endpoint.inl"
#endif
#endif // KBE_ENDPOINT_H
