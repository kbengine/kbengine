/*
 *  MSAutoReleasePool.h
 *  test
 *
 *  Created by admin on 12-1-6.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MS_AUTO_RELEASE_POOL_H_
#define _MS_AUTO_RELEASE_POOL_H_


//class NSAutoreleasePool;

class MSAutoReleasePool{
private:
	void * pool;
public:
	MSAutoReleasePool();
	~MSAutoReleasePool();
};
#endif
