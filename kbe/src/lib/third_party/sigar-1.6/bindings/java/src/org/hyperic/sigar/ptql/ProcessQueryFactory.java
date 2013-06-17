/*
 * Copyright (c) 2006-2008 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.hyperic.sigar.ptql;

import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;

public class ProcessQueryFactory {
    
    private static ProcessQueryFactory instance = null;

    private Map cache = new HashMap();

    public ProcessQueryFactory() {}

    public void clear() {
        for (Iterator it=this.cache.values().iterator();
             it.hasNext();)
        {
            SigarProcessQuery query = (SigarProcessQuery)it.next();
            query.destroy();
        }
        this.cache.clear();
    }

    public static ProcessQueryFactory getInstance() {
        if (instance == null) {
            instance = new ProcessQueryFactory();
        }
        return instance;
    }

    public ProcessQuery getQuery(String query)
        throws MalformedQueryException {

        if (query == null) {
            throw new MalformedQueryException("null query");
        }

        if (query.length() == 0) {
            throw new MalformedQueryException("empty query");
        }

        ProcessQuery pQuery = (ProcessQuery)this.cache.get(query);

        if (pQuery != null) {
            return pQuery;
        }

        pQuery = new SigarProcessQuery();
        ((SigarProcessQuery)pQuery).create(query);
        this.cache.put(query, pQuery);
        return pQuery;
    }

    /**
     * @deprecated
     * @see #getQuery(String)
     */
    public static ProcessQuery getInstance(String query)
        throws MalformedQueryException {

        return getInstance().getQuery(query);
    }
}
