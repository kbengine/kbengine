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

package org.hyperic.sigar.win32.test;

import org.hyperic.sigar.test.SigarTestCase;
import org.hyperic.sigar.win32.LocaleInfo;

public class TestLocaleInfo extends SigarTestCase {
    
    public TestLocaleInfo(String name) {
        super(name);
    }

    private void checkInfo(LocaleInfo info, String match)
        throws Exception {

        assertGtZeroTrace("id", info.getId());
        assertGtZeroTrace("primary lang", info.getPrimaryLangId());
        assertGtEqZeroTrace("sub lang", info.getSubLangId());
        assertLengthTrace("perflib id", info.getPerflibLangId());
        assertIndexOfTrace("lang",
                           info.toString(), match);
    }

    public void testInfo() throws Exception {
        Object[][] tests = {
            { new Integer(0x16), "Portuguese" },
            { new Integer(LocaleInfo.makeLangId(0x09,0x05)), "New Zealand" },
            { new Integer(0x07), "German" },
            { new Integer(LocaleInfo.makeLangId(0x0a,0x14)), "Puerto Rico" },
        };

        for (int i=0; i<tests.length; i++) {
            Integer id = (Integer)tests[i][0];
            String lang = (String)tests[i][1];
            LocaleInfo info = new LocaleInfo(id);
            checkInfo(info, lang);
        }

        checkInfo(new LocaleInfo(), "");
    }
}
