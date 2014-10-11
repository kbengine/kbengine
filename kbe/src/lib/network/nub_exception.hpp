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

#ifndef KBE_NETWORKNUB_EXCEPTION_HPP
#define KBE_NETWORKNUB_EXCEPTION_HPP

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
	NubException(Reason reason, const Address & addr = Address::NONE);
	virtual ~NubException() {};
	Reason reason() const;
	bool getAddress(Address & addr) const;

private:
	Reason		reason_;
	Address address_;
};

inline NubException::NubException(Reason reason, const Address & addr) :
	reason_(reason),
	address_(addr)
{}


inline Reason NubException::reason() const
{
	return reason_;
}


inline bool NubException::getAddress(Address & addr) const
{
	addr = address_;
	return address_ != Address::NONE;
}

}
}

#endif // KBE_NETWORKNUB_EXCEPTION_HPP
