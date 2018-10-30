// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 

INLINE uint64 Proxy::rndUUID() const
{
	return rndUUID_;
}

INLINE void Proxy::rndUUID(uint64 uid)
{
	rndUUID_ = uid;
}

INLINE COMPONENT_CLIENT_TYPE Proxy::getClientType() const
{
	return clientComponentType_;
}

INLINE void Proxy::setClientType(COMPONENT_CLIENT_TYPE ctype)
{
	clientComponentType_ = ctype;
}

INLINE const std::string& Proxy::getLoginDatas()
{
	return loginDatas_;
}

INLINE void Proxy::setLoginDatas(const std::string& datas)
{
	loginDatas_ = datas;
}

INLINE const std::string& Proxy::getCreateDatas()
{
	return createDatas_;
}

INLINE void Proxy::setCreateDatas(const std::string& datas)
{
	createDatas_ = datas;
}

INLINE void Proxy::addr(const Network::Address& address)
{ 
	addr_.ip = address.ip;
	addr_.port = address.port;
}

INLINE const Network::Address& Proxy::addr() const
{ 
	return addr_; 
}

INLINE bool Proxy::clientEnabled() const
{
	return clientEnabled_;
}

}
