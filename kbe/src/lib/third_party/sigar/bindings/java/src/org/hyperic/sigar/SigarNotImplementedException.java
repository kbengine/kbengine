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

/**
 * Sigar exception class, thrown for methods which are not implemented
 * on a given platform.
 */
public class SigarNotImplementedException extends SigarException {

    private static final String msg = 
        "This method has not been implemented on this platform";

    public static final SigarNotImplementedException INSTANCE =
        new SigarNotImplementedException(msg);

    public SigarNotImplementedException () { super(); }

    public SigarNotImplementedException (String s) { super(s); }
}
