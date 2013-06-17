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

package org.hyperic.sigar.ptql;

import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.hyperic.sigar.util.ReferenceMap;

public class StringPattern {
    private static Map patterns = ReferenceMap.synchronizedMap();
    
    /**
     * Wrapper around Pattern.compile(regex).matcher(source).find()
     */
    public static boolean matches(String source, String regex) {
        Pattern pattern = (Pattern)patterns.get(regex);
        if (pattern == null) {
            pattern = Pattern.compile(regex);
            patterns.put(regex, pattern);
        }
        Matcher matcher = pattern.matcher(source);
        return matcher.find();
    }
}

