/*
 *  Texture2DManager.mm
 *  test
 *
 *  Created by admin on 12-1-6.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */
#import <UIKit/UIKit.h>

#include "MSAutoReleasePool.h"



MSAutoReleasePool::MSAutoReleasePool(){
	pool = [[NSAutoreleasePool alloc] init];

}

MSAutoReleasePool::~MSAutoReleasePool(){
//	[pool drain];
	[(NSAutoreleasePool*)pool release];
}
