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

#ifndef __HTTP_CB_HANDLER__
#define __HTTP_CB_HANDLER__

#include "cstdkbe/cstdkbe.hpp"

namespace KBEngine{
namespace Mercury{
class EndPoint;
}

class HTTPCBHandler : public Mercury::InputNotificationHandler
{
public:
	HTTPCBHandler();
	virtual ~HTTPCBHandler();

	Mercury::EndPoint* pEndPoint(){ return pEndPoint_; }

	void onAccountActivated(std::string& code, bool success);
protected:
	
	struct CLIENT
	{
		KBEShared_ptr< Mercury::EndPoint > endpoint;
		uint8 state;
		std::string code;
	};

	virtual int handleInputNotification( int fd );

	Mercury::EndPoint* pEndPoint_;

	std::map< int, CLIENT > clients_;
};

}
#endif
