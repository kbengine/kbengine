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

package org.hyperic.sigar.pager;

/**
 * The default page processor does not process any elements
 * that you're paging.  This is useful if you're only looking
 * to page through an existing collection, and you don't need
 * to perform any transformations on the elements that are
 * found to belong in the resultant page.
 */
public class DefaultPagerProcessor implements PagerProcessor {

    /**
     * Default processor does not process anything, it just
     * returns what was passed in.
     * @param o The object to process.
     * @return The same (completely unmodified) object that was passed in.
     * @see PagerProcessor#processElement
     */
    public Object processElement(Object o) {
        return o;
    }
}
