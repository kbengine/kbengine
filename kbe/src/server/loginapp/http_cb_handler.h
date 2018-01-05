/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_HTTP_CB_HANDLER_H
#define KBE_HTTP_CB_HANDLER_H

#include "common/common.h"

namespace KBEngine{
namespace Network{
class EndPoint;
}

class HTTPCBHandler : public Network::InputNotificationHandler
{
public:
	HTTPCBHandler();
	virtual ~HTTPCBHandler();

	Network::EndPoint* pEndPoint(){ return pEndPoint_; }

	void onAccountActivated(std::string& code, bool success);
	void onAccountBindedEmail(std::string& code, bool success);
	void onAccountResetPassword(std::string& code, bool success);

protected:
	
	struct CLIENT
	{
		KBEShared_ptr< Network::EndPoint > endpoint;
		uint8 state;
		std::string code;
	};

	virtual int handleInputNotification( int fd );

	Network::EndPoint* pEndPoint_;

	std::map< int, CLIENT > clients_;
};

}

#endif // KBE_HTTP_CB_HANDLER_H
