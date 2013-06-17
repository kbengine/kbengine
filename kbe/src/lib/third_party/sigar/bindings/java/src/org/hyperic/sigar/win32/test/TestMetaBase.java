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

package org.hyperic.sigar.win32.test;

import junit.framework.TestCase;

public class TestMetaBase extends TestCase {

    public TestMetaBase(String name) {
        super(name);
    }

    /**
     * TODO: This test shouldn't fail if IIS is not installed.  If
     *       the exceptions thrown from the native code were actually
     *       useful, we could do this.  Re-add this test when the
     *       exceptions are cleaned up.
     **/
    public void testMetaBase() throws Exception {
    }
}
