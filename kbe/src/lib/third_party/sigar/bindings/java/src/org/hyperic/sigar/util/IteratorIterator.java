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

package org.hyperic.sigar.util;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * Iterator for multiple Iterators.
 */
public class IteratorIterator implements Iterator {

    private int ix = 0;
    private Iterator curr = null;

    private ArrayList iterators = new ArrayList();

    public IteratorIterator() { }

    public void add(Iterator iterator) {
        this.iterators.add(iterator);
    }

    public boolean hasNext() {
        int size = this.iterators.size();

        //first time through
        if (this.curr == null) {
            if (size == 0) {
                return false;
            }

            this.curr = (Iterator)this.iterators.get(0);
        }

        if (this.curr.hasNext()) {
            return true;
        }

        this.ix++;
        if (this.ix >= size) {
            return false;
        }

        this.curr = (Iterator)this.iterators.get(this.ix);

        //recurse in the event that this.curr is empty
        return hasNext();
    }

    public Object next() {
        return this.curr.next();
    }

    public void remove() {
        throw new UnsupportedOperationException();
    }
}
