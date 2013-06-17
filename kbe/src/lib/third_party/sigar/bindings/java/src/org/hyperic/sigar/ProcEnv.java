/*
 * Copyright (c) 2006 Hyperic, Inc.
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

package org.hyperic.sigar;

import java.util.Map;

/**
 * Lookup environment for a process.
 */
class ProcEnv {

    private ProcEnv () { }

    /**
     * @param sigar The Sigar object.
     * @param pid Process id.
     * @return Map of environment.
     * @exception SigarException on failure.
     * @see org.hyperic.sigar.Sigar#getProcEnv
     */
    public static native Map getAll(Sigar sigar, long pid)
        throws SigarException;

    /**
     * @param sigar The Sigar object.
     * @param pid Process id.
     * @param key Environment variable name.
     * @return Environment variable value.
     * @exception SigarException on failure.
     * @see org.hyperic.sigar.Sigar#getProcEnv
     */
    public static native String getValue(Sigar sigar, long pid, String key)
        throws SigarException;
}
