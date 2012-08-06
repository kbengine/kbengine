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


#ifndef __PROXY_H__
#define __PROXY_H__
	
// common include
#include "base.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{


namespace Mercury
{
class Channel;
}

class Proxy : public Base
{
	/** 子类化 将一些py操作填充进派生类 */
	BASE_SCRIPT_HREADER(Proxy, Base)	
public:
	Proxy(ENTITY_ID id, ScriptModule* scriptModule);
	~Proxy();
	
	/**
		这个entity被激活了, 在客户端初始化好对应的entity后， 这个方法被调用
	*/
	void onEntitiesEnabled(void);
	
	/**
		登陆尝试， 当正常的登陆失败之后， 调用这个接口再进行尝试 
	*/
	void onLogOnAttempt(std::string& addr, uint32& port, std::string& password);
	
	/** 
		当察觉这个entity对应的客户端socket断开时被调用 
	*/
	void onClientDeath(void);
	
	/** 
		当客户端所关联的这个entity的cell被创建时，被调用 
	*/
	void onClientGetCell(void);

	/**
		每个proxy创建之后都会由系统产生一个uuid， 提供提供前端重登陆时用作身份识别
	*/
	uint64 rndUUID()const{ return rndUUID_; }
	void rndUUID(uint64 uid){ rndUUID_ = uid; }
public:
	/** 将其自身所关联的客户端转给另一个proxy去关联 */
	void giveClientTo(Proxy* proxy);
	void onGiveClientToMe(Mercury::Channel* lpChannel);
	DECLARE_PY_MOTHOD_ARG1(pyGiveClientTo, PyObject_ptr);
protected:
	uint64 rndUUID_;
};

}
#endif
