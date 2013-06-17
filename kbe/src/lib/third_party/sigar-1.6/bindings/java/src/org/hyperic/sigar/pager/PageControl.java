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

import java.io.Serializable;

/**
 * A utility class to wrap up all the paging/sorting options that
 * are frequently used with finders and other methods that return
 * lists of things.
 */
public class PageControl implements Serializable, Cloneable {
    public static final int SIZE_UNLIMITED = -1;

    public static final int SORT_UNSORTED = 0;
    public static final int SORT_ASC      = 1;
    public static final int SORT_DESC     = 2;

    private int pagenum       = 0;
    private int pagesize      = SIZE_UNLIMITED;
    private int sortorder     = SORT_UNSORTED;
    private int sortattribute = SortAttribute.DEFAULT;

    private Serializable metaData;  // Meta-data that PageLists have returned


    public PageControl() {}

    public PageControl(int pagenum, int pagesize) {
        this.pagenum  = pagenum;
        this.pagesize = pagesize;
    }

    public PageControl(int pagenum, int pagesize, 
                       int sortorder, int sortattribute) {
        this.pagenum       = pagenum;
        this.pagesize      = pagesize;
        this.sortorder     = sortorder;
        this.sortattribute = sortattribute;
    }

    public boolean isAscending() {
        return this.sortorder == SORT_ASC;
    }

    public boolean isDescending() {
        return this.sortorder == SORT_DESC;
    }

    /**
     * sets the initial defaults for the PageControl.  Sort attribute specifies
     * which attribute to sort on.
     * 
     * @param pc
     * @param defaultSortAttr specifies the attribute to sort on.
     * @return PageControl
     */
    public static PageControl initDefaults(PageControl pc, 
                                           int defaultSortAttr) {
        if (pc == null) {
            pc = new PageControl();
        }
        else {
            pc = (PageControl) pc.clone();
        }
        
        if (pc.getSortattribute() == SortAttribute.DEFAULT) {
            pc.setSortattribute(defaultSortAttr);
        }
        if (pc.getSortorder() == SORT_UNSORTED) {
            pc.setSortorder(SORT_ASC);
        }

        return pc;
    }

    /** @return The current page number (0-based) */
    public int getPagenum() {
        return this.pagenum;
    }

    /** @param pagenum Set the current page number to <code>pagenum</code> */
    public void setPagenum(int pagenum) {
        this.pagenum = pagenum;
    }

    /** @return The current page size */
    public int getPagesize() {
        return this.pagesize;
    }

    /** @param pagesize Set the current page size to this value */
    public void setPagesize(int pagesize) {
        this.pagesize = pagesize;
    }

    /** @return The sort order used.  This is one of the SORT_XXX constants. */
    public int getSortorder() {
        return this.sortorder;
    }

    /** @param sortorder Sort order to use, one of the SORT_XXX constants. */
    public void setSortorder(int sortorder) {
        this.sortorder = sortorder;
    }

    /** @return The attribute that the sort is based on. */
    public int getSortattribute() {
        return this.sortattribute;
    }

    /** @param attr Set the attribute that the sort is based on. */
    public void setSortattribute(int attr) {
        this.sortattribute = attr;
    }

    public Serializable getMetaData() {
        return this.metaData;
    }

    public void getMetaData(Serializable metaData) {
        this.metaData = metaData;
    }

    /**
     * Get the index of the first item on the page as dictated by the
     * page size and page number.  
     */
    public int getPageEntityIndex() {
        return this.pagenum * this.pagesize;
    }

    public String toString() {
        StringBuffer s = new StringBuffer("{");
        s.append("pn=" + this.pagenum + " ");
        s.append("ps=" + this.pagesize + " ");
        
        s.append("so=");
        
        switch(this.sortorder) {
          case SORT_ASC:
            s.append("asc ");
            break;
          case SORT_DESC:
            s.append("desc");
            break;
          case SORT_UNSORTED:
            s.append("unsorted ");
            break;
          default:
            s.append(' ');
        }
        
        s.append("sa=" + this.sortattribute + " ");
        s.append("}");

        return s.toString();
    }
    
    public Object clone() {
        PageControl res =
            new PageControl(this.pagenum, this.pagesize,
                            this.sortorder, this.sortattribute);
        res.metaData = metaData;
        return res;
    }
}
