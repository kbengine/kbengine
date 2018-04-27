// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
