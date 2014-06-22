//
//  KNetworkInterface.h
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#ifndef __libKBEClient__KNetworkInterface__
#define __libKBEClient__KNetworkInterface__

#include <iostream>
#include "KBEClientcore.h"


namespace KBEngineClient
{
	class KNetworkInterface{
	public:
		void Connect();
		void send(uint8* data,int size);
		void recv();
		void bindMessage();
	};
}
#endif /* defined(__libKBEClient__KNetworkInterface__) */
