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

#include "apr_arch_threadproc.h"

static struct beos_key key_table[BEOS_MAX_DATAKEYS];
static struct beos_private_data *beos_data[BEOS_MAX_DATAKEYS];
static sem_id lock;

APR_DECLARE(apr_status_t) apr_threadkey_private_create(apr_threadkey_t **key,
                                       void (*dest)(void *), apr_pool_t *pool)
{
    (*key) = (apr_threadkey_t *)apr_palloc(pool, sizeof(apr_threadkey_t));
    if ((*key) == NULL) {
        return APR_ENOMEM;
    }

    (*key)->pool = pool;
    	
	acquire_sem(lock);
	for ((*key)->key=0; (*key)->key < BEOS_MAX_DATAKEYS; (*key)->key++){
		if (key_table[(*key)->key].assigned == 0){
			key_table[(*key)->key].assigned = 1;
			key_table[(*key)->key].destructor = dest;
			release_sem(lock);
			return APR_SUCCESS;
		}				

	}
	release_sem(lock);
    return APR_ENOMEM;
}

APR_DECLARE(apr_status_t) apr_threadkey_private_get(void **new, apr_threadkey_t *key)
{
	thread_id tid;
	int i, index=0;
	tid = find_thread(NULL);
	for (i=0;i<BEOS_MAX_DATAKEYS;i++){
		if (beos_data[i]->data){
			/* it's been used */
			if (beos_data[i]->td == tid){
				index = i;
			}
		}
	}
	if (index == 0){
		/* no storage for thread so we can't get anything... */
		return APR_ENOMEM;
	}

	if ((key->key < BEOS_MAX_DATAKEYS) && (key_table)){
		acquire_sem(key_table[key->key].lock);
		if (key_table[key->key].count){
			(*new) = (void*)beos_data[index]->data[key->key];
		} else {
			(*new) = NULL;
		}
		release_sem(key_table[key->key].lock);
	} else {
		(*new) = NULL;
	}
	return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_threadkey_private_set(void *priv, apr_threadkey_t *key)
{
	thread_id tid;
	int i,index = 0, ret = 0;

	tid = find_thread(NULL);	
	for (i=0; i < BEOS_MAX_DATAKEYS; i++){
		if (beos_data[i]->data){
			if (beos_data[i]->td == tid){index = i;}
		}
	}
	if (index==0){
		/* not yet been allocated */
		for (i=0; i< BEOS_MAX_DATAKEYS; i++){
			if (! beos_data[i]->data){
				/* we'll take this one... */
				index = i;
				beos_data[i]->data = (const void **)malloc(sizeof(void *) * BEOS_MAX_DATAKEYS);
				memset((void *)beos_data[i]->data, 0, sizeof(void *) * BEOS_MAX_DATAKEYS);
				beos_data[i]->count = (int)malloc(sizeof(int));
				beos_data[i]->td = (thread_id)malloc(sizeof(thread_id));
				beos_data[i]->td = tid;
			}
		}
	}
	if (index == 0){
		/* we're out of luck.. */
		return APR_ENOMEM;
	}
	if ((key->key < BEOS_MAX_DATAKEYS) && (key_table)){
		acquire_sem(key_table[key->key].lock);
		if (key_table[key->key].count){
			if (beos_data[index]->data[key->key] == NULL){
				if (priv != NULL){
					beos_data[index]->count++;
					key_table[key->key].count++;
				}
			} else {
				if (priv == NULL){
					beos_data[index]->count--;
					key_table[key->key].count--;
				}
			}
			beos_data[index]->data[key->key] = priv;
			ret = 1;
		} else {
			ret = 0;
		}
		release_sem(key_table[key->key].lock);
	}
	if (ret)
    	return APR_SUCCESS;
	return APR_ENOMEM;
}

APR_DECLARE(apr_status_t) apr_threadkey_private_delete(apr_threadkey_t *key)
{
	if (key->key < BEOS_MAX_DATAKEYS){
		acquire_sem(key_table[key->key].lock);
		if (key_table[key->key].count == 1){
			key_table[key->key].destructor = NULL;
			key_table[key->key].count = 0;
		}
		release_sem(key_table[key->key].lock);
	} else {
		return APR_ENOMEM;
	}
	return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_threadkey_data_get(void **data, const char *key,
                                                 apr_threadkey_t *threadkey)
{
    return apr_pool_userdata_get(data, key, threadkey->pool);
}

APR_DECLARE(apr_status_t) apr_threadkey_data_set(void *data, const char *key,
                                                 apr_status_t (*cleanup) (void *),
                                                 apr_threadkey_t *threadkey)
{
    return apr_pool_userdata_set(data, key, cleanup, threadkey->pool);
}

APR_DECLARE(apr_status_t) apr_os_threadkey_get(apr_os_threadkey_t *thekey, apr_threadkey_t *key)
{
    *thekey = key->key;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_threadkey_put(apr_threadkey_t **key, 
                                               apr_os_threadkey_t *thekey, apr_pool_t *pool)
{
    if (pool == NULL) {
        return APR_ENOPOOL;
    }
    if ((*key) == NULL) {
        (*key) = (apr_threadkey_t *)apr_pcalloc(pool, sizeof(apr_threadkey_t));
        (*key)->pool = pool;
    }
    (*key)->key = *thekey;
    return APR_SUCCESS;
}
