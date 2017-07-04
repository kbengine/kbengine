/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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
	addr_ = address; 
}

INLINE const Network::Address& Proxy::addr() const
{ 
	return addr_; 
}

INLINE bool Proxy::entitiesEnabled() const
{ 
	return entitiesEnabled_;
}

}
