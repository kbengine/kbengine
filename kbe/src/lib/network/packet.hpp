/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __SOCKETPACKET_H__
#define __SOCKETPACKET_H__
	
// common include
#include "cstdkbe/cstdkbe.hpp"
#include "memorystream.hpp"
#include "opcodes.hpp"	
//#define NDEBUG
#include <assert.h>
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

class Packet : public MemoryStream
{
public:
    Packet(): 
    MemoryStream(0), 
    m_opcode_(0),
    m_onRecvtime_(0)
    {
    }
    
    explicit Packet(OPCODE_TYPE opcode, size_t res = 200): 
    MemoryStream(res), m_opcode_(opcode) 
    { 
		(*this) << (OPCODE_TYPE)m_opcode_;
    }

    Packet(const Packet &packet): 
    MemoryStream(packet), 
    m_opcode_(packet.m_opcode_)
    {
    }

    void initialize(OPCODE_TYPE opcode, size_t newres = 200)
    {
        clear();
        m_storage_.resize(newres);
        m_opcode_ = opcode;
		(*this) << (OPCODE_TYPE)m_opcode_;
    }

    void unPacket(const char* data, size_t& length)
    {
		append(data, length);
		(*this) >> m_opcode_;
		m_onRecvtime_ = getSystemTime();
    }

	virtual ~Packet(void){}	

    OPCODE_TYPE getOpcode() const { return m_opcode_; }

    void setOpcode(OPCODE_TYPE opcode) { m_opcode_ = opcode; }

	uint32 getOnRecvTime(void)const{ return m_onRecvtime_; }
protected:
	OPCODE_TYPE		m_opcode_;									// 数据包的操作码
	uint32			m_onRecvtime_;								// 这个数据包接收时的时间
};

}
#endif
