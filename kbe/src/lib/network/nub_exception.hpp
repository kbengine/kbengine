/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORKNUB_EXCEPTION__
#define __NETWORKNUB_EXCEPTION__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/address.hpp"

namespace KBEngine { 
namespace Mercury
{
class NubException
{
public:
	NubException( Reason reason, const Address & addr = Address::NONE );
	virtual ~NubException() {};
	Reason reason() const;
	bool getAddress( Address & addr ) const;

private:
	Reason		reason_;
	Address address_;
};

inline NubException::NubException( Reason reason, const Address & addr ) :
	reason_( reason ),
	address_( addr )
{}


inline Reason NubException::reason() const
{
	return reason_;
}


inline bool NubException::getAddress( Address & addr ) const
{
	addr = address_;
	return address_ != Address::NONE;
}

}
}

#endif // __NETWORKNUB_EXCEPTION__