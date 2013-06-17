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
 * Provides a point of extensibility in the paging behavior.
 * If you supply a PagerProcessor when you get a Pager,
 * then that processor will be called to process each element
 * as the pager moves it from the source collection to the
 * destination collection.
 */
public interface PagerProcessor {

    /**
     * Process an element as the pager moves it from the source 
     * collection to the destination collection.
     * @param o The object to process.
     * @return The processed object.
     */
    public Object processElement(Object o);
}
