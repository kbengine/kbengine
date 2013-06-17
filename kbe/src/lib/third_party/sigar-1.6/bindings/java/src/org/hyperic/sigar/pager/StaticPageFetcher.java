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

import java.util.Arrays;
import java.util.List;

/**
 * A fetcher which uses a static array of strings to page
 * through.
 */

public class StaticPageFetcher extends PageFetcher {

    private List data;

    public StaticPageFetcher(String[] data) {
        this.data = Arrays.asList(data);
    }

    public StaticPageFetcher(List data) {
        this.data = data;
    }

    public PageList getPage(PageControl control)
        throws PageFetchException
    {
        PageList res = new PageList();
        int startIdx, endIdx;

        res.setTotalSize(this.data.size());

        if (control.getPagesize() == PageControl.SIZE_UNLIMITED ||
            control.getPagenum() == -1)
        {
            res.addAll(this.data);
            return res;
        }

        startIdx = control.getPageEntityIndex();
        endIdx   = startIdx + control.getPagesize();

        if (startIdx >= this.data.size()) {
            return res;
        }

        if (endIdx > this.data.size()) {
            endIdx = this.data.size();
        }

        res.addAll(this.data.subList(startIdx, endIdx));

        return res;
    }
}
