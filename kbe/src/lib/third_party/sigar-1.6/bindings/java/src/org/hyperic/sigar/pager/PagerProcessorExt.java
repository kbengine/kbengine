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
public interface PagerProcessorExt extends PagerProcessor {

    /**
     * Get the event handler for this pager. May return null to indicate
     * that no event handler should be used.
     */
    public PagerEventHandler getEventHandler();

    /**
     * Determines if null values are included in the Pager's results.
     * @return If this method returns true, then when the processElement
     * method returns null, that element will not be included in the results.
     * If this methods returns false, then nulls may be added to the result
     * page.
     */
    public boolean skipNulls();

    /**
     * Process an element as the pager moves it from the source 
     * collection to the destination collection. This version
     * allows an additional argument to be passed along.
     * @param o1 The object to process.
     * @param o2 Additional data required to processElement.
     * @return The processed object.
     */
    public Object processElement(Object o1, Object o2);

}
