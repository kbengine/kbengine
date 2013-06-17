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

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * Implements a generic pager.  What is a pager?  Let's say you
 * have a large collection of objects, perhaps a long list of 
 * EJB Local Interfaces.  You're interested in breaking the
 * mammoth list out into a number pages, each with 25 items on it.
 * You're interested in returning page #17 of such a collection.
 * Why bother implementing the "skip past a bunch of things, then
 * return pagesize items in the resultant colleciton" over and over 
 * again.
 *
 * You can also have the elements go through a processor that you
 * supply as they move from the source collection to the 
 * destination collection.
 */
public class Pager {

    public static final String DEFAULT_PROCESSOR_CLASSNAME
        = "org.hyperic.sigar.pager.DefaultPagerProcessor";

    private static Map PagerProcessorMap = 
        Collections.synchronizedMap(new HashMap());
    private PagerProcessor processor = null;
    private boolean skipNulls = false;
    private PagerEventHandler eventHandler = null;

    private Pager ( PagerProcessor processor ) {

        this.processor = processor;
        this.skipNulls = false;
        this.eventHandler = null;

        if ( this.processor instanceof PagerProcessorExt ) {
            this.skipNulls = ((PagerProcessorExt) this.processor).skipNulls();
            this.eventHandler
                = ((PagerProcessorExt) this.processor).getEventHandler();
        }
    }

    public static Pager getDefaultPager () {
        try {
            return getPager(DEFAULT_PROCESSOR_CLASSNAME);
        } catch ( Exception e ) {
            throw new IllegalStateException("This should never happen: " + e);
        }
    }

    /**
     * Get a pager based on the PagerProcessor supplied.
     */
    public static Pager getPager (String pageProcessorClassName) 
        throws InstantiationException, IllegalAccessException, 
               ClassNotFoundException 
    {
        Pager p = null;
        p = (Pager) PagerProcessorMap.get(pageProcessorClassName);
        if ( p == null ) {
            PagerProcessor processor = (PagerProcessor) 
                Class.forName(pageProcessorClassName).newInstance();
            p = new Pager(processor);
            PagerProcessorMap.put(pageProcessorClassName, p);
        }
        return p;
    }

    /**
     * Seek to the specified pagenum in the source collection and
     * return pagsize numberof of elements in the List.
     * If pagenum or pagesize is -1, then everything in the
     * source collection will be returned.
     * 
     * @param source The source collection to seek through.
     * @param pagenum The page number to seek to.  If there not
     * enough pages in the collection, then an empty list will be returned.
     * @param pagesize The size of each page.
     * @return PageList containing results of seek.
     */
    public PageList seek ( Collection source, int pagenum, int pagesize ) {
        return seek(source,pagenum,pagesize,null);
    }

    /**
     * Seek to the specified pagenum in the source collection and return pagsize
     * numberof of elements in the List, as specified the PageControl object.
     * If pagenum or pagesize is -1, then everything in the
     * source collection will be returned.
     * 
     * @param source The source collection to seek through.
     * @param pc The page control used for page size and page number to seek to.  If there not
     * enough pages in the collection, then an empty list will be returned.
     * @return PageList containing results of seek.
     */
    public PageList seek ( Collection source, PageControl pc ) {
        if (pc == null)
            pc = new PageControl();
        
        return seek(source, pc.getPagenum(), pc.getPagesize(), null);
    }

    /**
     * Seek to the specified pagenum in the source collection and
     * return pagsize numberof of elements in the List.
     * If pagenum or pagesize is -1, then everything in the
     * source collection will be returned.
     * 
     * @param source The source collection to seek through.
     * @param pagenum The page number to seek to.  If there not
     * enough pages in the collection, then an empty list will be returned.
     * @param pagesize The size of each page.
     * @param procData - any data object required by the processor.
     * @return PageList containing results of seek.
     */
    public PageList seek ( Collection source, int pagenum, int pagesize,
                           Object procData ) {

        PageList dest = new PageList();
        seek(source, dest, pagenum, pagesize, procData);
        dest.setTotalSize(source.size());
        return dest;
    }

    /**
     * Seek to the specified pagenum in the source collection and place
     * pagesize number of elements into the dest collection.
     * If pagenum or pagesize is -1, then everything in the
     * source collection will be placed in the dest collection.
     * 
     * @param source The source collection to seek through.
     * @param dest The collection to place results into.
     * @param pagenum The page number to seek to.  If there not
     * enough pages in the collection, then an empty list will be returned.
     * @param pagesize The size of each page.
     */
    public void seek ( Collection source, Collection dest, int pagenum,
                       int pagesize ) {
        seek(source,dest,pagenum,pagesize,null);
    }
    /**
     * Seek to the specified pagenum in the source collection and place
     * pagesize number of elements into the dest collection.
     * If pagenum or pagesize is -1, then everything in the
     * source collection will be placed in the dest collection.
     * 
     * @param source The source collection to seek through.
     * @param dest The collection to place results into.
     * @param pagenum The page number to seek to.  If there not
     * enough pages in the collection, then an empty list will be returned.
     * @param pagesize The size of each page.
     * @param procData any object required to process the item.
     */
    public void seek ( Collection source, Collection dest, int pagenum,
                       int pagesize, Object procData) {

        Iterator iter = source.iterator();
        int i, currentPage;

        if ( pagesize == -1 || pagenum == -1 ) {
            pagenum = 0;
            pagesize = Integer.MAX_VALUE;
        }

        for ( i=0, currentPage=0;
              iter.hasNext() && currentPage < pagenum;
              i++, currentPage += (i % pagesize == 0) ? 1:0 ) {
            iter.next();
        }

        if ( this.eventHandler != null ) this.eventHandler.init();

        if ( this.skipNulls ) {
            Object elt;
            while ( iter.hasNext() && dest.size() < pagesize ) {
                if (this.processor instanceof PagerProcessorExt)
                    elt = ((PagerProcessorExt)this.processor)
                      .processElement(iter.next(), procData);
                else
                    elt = this.processor.processElement(iter.next());
                if ( elt == null )
                    continue;
                dest.add(elt);
            }
        } else {
            while ( iter.hasNext() && dest.size() < pagesize ) {
                dest.add(this.processor.processElement(iter.next()));
            }
        }

        if ( this.eventHandler != null ) this.eventHandler.cleanup();
    }

    /**
     * Seek to the specified pagenum in the source collection and place
     * pagesize number of elements into the dest collection. Unlike,
     * seek(), all items are passed to the Processor or ProcessorExt
     * regardless whether they are placed in dest collection. If pagenum
     * or pagesize is -1, then everything in the source collection will
     * be placed in the dest collection.
     *
     * @param source The source collection to seek through.
     * @param pagenum The page number to seek to.  If there not
     * enough pages in the collection, then an empty list will be returned.
     * @param pagesize The size of each page.
     * @param procData any object required to process the item.
     */
    public PageList seekAll ( Collection source, int pagenum, int pagesize,
                              Object procData ) {

        PageList dest = new PageList();
        seekAll(source, dest, pagenum, pagesize, procData);
        dest.setTotalSize(source.size());
        return dest;
    }

    /**
     * Seek to the specified pagenum in the source collection and place
     * pagesize number of elements into the dest collection. Unlike,
     * seek(), all items are passed to the Processor or ProcessorExt 
     * regardless whether they are placed in dest collection. If pagenum 
     * or pagesize is -1, then everything in the source collection will 
     * be placed in the dest collection.
     * 
     * @param source The source collection to seek through.
     * @param dest The collection to place results into.
     * @param pagenum The page number to seek to.  If there not
     * enough pages in the collection, then an empty list will be returned.
     * @param pagesize The size of each page.
     * @param procData any object required to process the item.
     */
    public void seekAll ( Collection source, Collection dest, int pagenum,
                          int pagesize, Object procData) {

        Iterator iter = source.iterator();
        int i, currentPage;

        if ( pagesize == -1 || pagenum == -1 ) {
            pagenum = 0;
            pagesize = Integer.MAX_VALUE;
        }

        // PR:8434 : Multi-part paging fixes.
        // 1.) Invoke the pager processor external which in many cases may
        //     be keeping track of element [in|ex]clusion.
        // 2.) The counter 'i' is used with modulus arithmetic to determine
        //     which page we should be on. Don't increment it if the proc-ext
        //     indicated that the element should not be paged.
        // 3.) 'i' begins it's existance at 0. Zero modulus anything yields
        //      zero. So the ternary expression needs to check for this initial
        //      condition and not increment the page number.
        for ( i=0, currentPage=0;
              iter.hasNext() && currentPage < pagenum;
              currentPage += (i != 0 && i % pagesize == 0) ? 1:0 ) {
            if (this.processor instanceof PagerProcessorExt) {
                Object ret = ((PagerProcessorExt)this.processor)
                    .processElement(iter.next(), procData);
                if (ret != null) {
                    i++;
                } 
            } else {
                this.processor.processElement(iter.next());
                i++;
            }
        }

        if ( this.eventHandler != null ) this.eventHandler.init();

        if ( this.skipNulls ) {
            Object elt;
            while ( iter.hasNext() ) {
                if (this.processor instanceof PagerProcessorExt)
                    elt = ((PagerProcessorExt)this.processor)
                      .processElement(iter.next(), procData);
                else
                    elt = this.processor.processElement(iter.next());
                if ( elt == null )
                    continue;

                if (dest.size() < pagesize)
                    dest.add(elt);
            }
        } else {
            while ( iter.hasNext() ) {
                Object elt = this.processor.processElement(iter.next());
                if (dest.size() < pagesize)
                    dest.add(elt);
            }
        }

        if ( this.eventHandler != null ) this.eventHandler.cleanup();
    }
    
    /** Process all objects in the source page list and return the destination
     * page list with the same total size
     */
    public PageList processAll(PageList source) {
        PageList dest = new PageList();
        for (Iterator it = source.iterator(); it.hasNext(); ) {
            Object elt = this.processor.processElement(it.next());
            if ( elt == null )
                continue;
            dest.add(elt);
        }
        
        dest.setTotalSize(source.getTotalSize());
        return dest;
    }
}
