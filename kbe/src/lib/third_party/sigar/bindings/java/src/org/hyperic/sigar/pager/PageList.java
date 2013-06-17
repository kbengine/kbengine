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
import java.util.ArrayList;
import java.util.Collection;

/**
 * A utility class that contains all a "page" of data that is viewable
 * <br>
 * this list may or may not conain the entire list of information. 
 * generally this list conains a subset of data. 
 * <br>
 * ex. say we have a list of 5000 users. the entire list does not need to be 
 * returned to only display the first 15 items, the user is only going to see 
 * the first 15 both the user and the application the user will want to know 
 * that there are 5000 users in the system.
 * <br> 
 * 
 */
public class PageList extends ArrayList implements Serializable {
    private int          totalSize = 0;
    private boolean      isUnbounded;   // Is the total size of the list known?
    private Serializable metaData;

    public PageList() {
        super();
        this.isUnbounded = false;
    }

    public PageList(Collection c, int totalSize) {
        super(c);
        this.totalSize   = totalSize;
        this.isUnbounded = false;
    }
    
    public String toString() {
        StringBuffer s = new StringBuffer("{");

        s.append("totalSize=" + totalSize + " ");
        s.append("}");
        return super.toString() + s.toString();

    }
    
    /** returns the total size of the "masterlist" that this page is a 
     *  subset of.
     * @return Value of property listSize.
     */
    public int getTotalSize() {
        return Math.max(this.size(), this.totalSize);
    }
    
    /** Sets the total size of the "masterlist" that this page is a subset of.
     * @param totalSize New value of property listSize.
     *
     */
    public void setTotalSize(int totalSize) {
        this.totalSize = totalSize;
    }

    public void setMetaData(Serializable metaData){
        this.metaData = metaData;
    }

    public Serializable getMetaData(){
        return this.metaData;
    }

    public boolean isUnbounded(){
        return this.isUnbounded;
    }

    public void setUnbounded(boolean isUnbounded){
        this.isUnbounded = isUnbounded;
    }
}
