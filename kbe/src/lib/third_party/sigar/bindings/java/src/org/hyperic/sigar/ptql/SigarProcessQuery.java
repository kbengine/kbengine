/*
 * Copyright (c) 2007 Hyperic, Inc.
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

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;

public class SigarProcessQuery implements ProcessQuery {
    int sigarWrapper = 0; //holds the sigar_ptql_query_t *
    long longSigarWrapper = 0; //same, but where sizeof(void*) > sizeof(int)

    native void create(String ptql)
        throws MalformedQueryException;

    native void destroy();

    protected void finalize() {
        destroy();
    }

    public native boolean match(Sigar sigar, long pid) 
        throws SigarException;

    public native long findProcess(Sigar sigar)
        throws SigarException;

    public native long[] find(Sigar sigar)
        throws SigarException;

    static boolean re(String haystack, String needle) {
        if (haystack == null) {
            return false;
        }
        if (needle == null) {
            return false;
        }
        return StringPattern.matches(haystack, needle);
    }
}
