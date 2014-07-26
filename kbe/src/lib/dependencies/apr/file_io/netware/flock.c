/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <nks/fsio.h>
#include "apr_arch_file_io.h"


apr_status_t apr_file_lock(apr_file_t *thefile, int type)
{
	int fc;

	fc = (type & APR_FLOCK_NONBLOCK) ? NX_RANGE_LOCK_TRYLOCK : NX_RANGE_LOCK_CHECK;

    if(NXFileRangeLock(thefile->filedes,fc, 0, 0) == -1)
		return errno;
            
    return APR_SUCCESS;
}

apr_status_t apr_file_unlock(apr_file_t *thefile)
{
    if(NXFileRangeUnlock(thefile->filedes,NX_RANGE_LOCK_CANCEL,0 , 0) == -1)
		return errno;
   
    return APR_SUCCESS;
}
