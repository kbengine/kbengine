/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
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

package org.hyperic.sigar.jmx;

/*
 * yeah, yeah, we could generate this class via xdoclet,
 * whoopdi-friggin-do... by hand is much less pain.
 */

public interface SigarProcessMBean {

    public Long getMemSize();

    /**
     * @deprecated
     * @see getMemSize
     */
    public Long getMemVsize();

    public Long getMemResident();

    public Long getMemShare();

    public Long getMemPageFaults();

    public Long getTimeUser();

    public Long getTimeSys();

    public Double getCpuUsage();

    public Long getOpenFd();
}
