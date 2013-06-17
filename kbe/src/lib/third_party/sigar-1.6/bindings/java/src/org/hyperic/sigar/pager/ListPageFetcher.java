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

import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.ListIterator;

/**
 * A PageFetcher which works with a pre-fetched list as the 
 * data backing the fetcher.
 */

public class ListPageFetcher extends PageFetcher {
    private List data;
    private int  sortOrder;

    public ListPageFetcher(List data) {
        super();
        this.data      = data;
        this.sortOrder = PageControl.SORT_UNSORTED;
    }

    public PageList getPage(PageControl control) {
        PageList res = new PageList();
        int startIdx, curIdx, endIdx;

        if (this.data.size() == 0) {
            return new PageList();
        }

        this.ensureSortOrder(control);
        res.setTotalSize(this.data.size());

        startIdx = clamp(control.getPageEntityIndex(), 0, 
                         this.data.size() - 1);
        curIdx   = startIdx;

        if (control.getPagesize() == PageControl.SIZE_UNLIMITED) {
            endIdx = this.data.size();
        }
        else {
            endIdx = clamp(startIdx + control.getPagesize(), startIdx,
                           this.data.size());
        }

        for (ListIterator i=this.data.listIterator(startIdx); 
            i.hasNext() && curIdx < endIdx; 
            curIdx++)
        {
            res.add(i.next());
        }

        return res;
    }

    private class DescSorter implements Comparator {
        public int compare(Object o1, Object o2){
            return -((Comparable)o1).compareTo((Comparable)o2);
        }

        public boolean equals(Object other){
            return false;
        }
    }

    private void ensureSortOrder(PageControl control) {
        if (control.getSortorder() == this.sortOrder) {
            return;
        }
        
        this.sortOrder = control.getSortorder();
        if (this.sortOrder == PageControl.SORT_UNSORTED) {
            return;
        }
        else if(this.sortOrder == PageControl.SORT_ASC) {
            Collections.sort(data);
        }
        else if(this.sortOrder == PageControl.SORT_DESC) {
            Collections.sort(data, new DescSorter());
        }
        else {
            throw new IllegalStateException("Unknown control sorting type: " +
                                            this.sortOrder);
        }
    }

    /**
     * Clamp a value to a range.  If the passed value is 
     * less than the minimum, return the minimum.  If it
     * is greater than the maximum, assign the maximum.
     * else return the passed value.
     */
    private static int clamp(int val, int min, int max) {
        return (int)clamp((long)val, (long)min, (long)max);
    }

    private static long clamp(long val, long min, long max) {
        if (val < min) {
            return min;
        }
        
        if (val > max) {
            return max;
        }

        return val;
    }
}
